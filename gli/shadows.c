#include "game_vars.h"
#include <stdlib.h>
#include <string.h>

#include "gli.h"
#include "handles.h"
#include "render.h"

#define CURRENT_HEAP GAME_VAR(gp_6F74)
#define MAX_QUEUED 100

typedef struct ShadowCamera {
    u32 handle;
    Texture texture;
} ShadowCamera;

typedef struct ShadowSize {
    int side;
    u32 count, next;
    ShadowCamera *cameras;
    GuestPtr camera_list, texture_list;
} ShadowSize;

static ShadowSize g_sizes[2] = {{.side = 128}, {.side = 64}};

static GuestPtr game_alloc(u32 size) {
    return AI_fn_p_vTrueAlloc(*GUEST(GuestPtr, CURRENT_HEAP), size);
}

static void game_free(GuestPtr block) {
    inv_MEM_fn_vFreeInHeap(*GUEST(GuestPtr, CURRENT_HEAP), block);
}

static u32 create_cameras(ShadowSize *z, u32 count) {
    z->camera_list = game_alloc(count << 2);
    z->texture_list = game_alloc(count << 2);
    z->cameras = calloc(count ? count : 1, sizeof *z->cameras);
    for (u32 i = 0; i < count; i++) {
        ShadowCamera *c = &z->cameras[i];
        c->texture = (Texture){.image = render_target_new(z->side, z->side), .address_u = ADDRESS_WRAP,
                               .address_v = ADDRESS_WRAP};
        c->texture.handle = handle_new(OBJECT_TEXTURE, &c->texture);
        c->handle = handle_new(OBJECT_CAMERA, c);
        GUEST(u32, z->camera_list)[i] = c->handle;
        GUEST(u32, z->texture_list)[i] = c->texture.handle;
    }
    return count;
}

s32 gli_shadows_open(u32 count128, u32 count64) {
    g_sizes[0].count = create_cameras(&g_sizes[0], count128);
    g_sizes[1].count = create_cameras(&g_sizes[1], count64);
    return g_sizes[0].count == count128 && g_sizes[1].count == count64;
}

void gli_shadows_close(void) {
    for (int k = 0; k < 2; k++) {
        ShadowSize *z = &g_sizes[k];
        for (u32 i = 0; i < z->count; i++) {
            handle_free(z->cameras[i].texture.handle);
            render_texture_free(z->cameras[i].texture.image);
            handle_free(z->cameras[i].handle);
            GUEST(u32, z->camera_list)[i] = 0;
            GUEST(u32, z->texture_list)[i] = 0;
        }
        free(z->cameras);
        z->cameras = NULL;
        if (z->camera_list) game_free(z->camera_list);
        if (z->texture_list) game_free(z->texture_list);
        z->camera_list = z->texture_list = 0;
        z->count = 0;
    }
}

void gli_shadows_begin(void) { g_sizes[0].next = g_sizes[1].next = 0; }

void gli_shadows_end(void) {}

typedef struct Queued {
    Atomic *atomic;
    Frame frame;
} Queued;

static Queued g_queued[MAX_QUEUED];
static int g_nqueued;
static ShadowView g_view;

static GliTexture *material_texture(GuestPtr material) {
    GuestPtr frame = game_stack_push(16);
    GLI_xGetMaterialTexture(material, frame);
    GuestPtr texture = *GUEST(GuestPtr, frame);
    game_stack_pop(16);
    return texture ? GUEST(GliTexture, texture) : NULL;
}

static void shadow_object(GuestPtr object) {
    const GeometricObject *go = GUEST(GeometricObject, object);
    const DriverObject *d = GUEST(DriverObject, go->driver_object);
    const s16 count = (s16)go->element_count;
    if (count == 0) return;
    int mesh = 0;
    for (int e = 0; (u32)e < (u32)count; e++) {
        if (GUEST(u8, go->element_types)[e * 2] != ELEMENT_MESH) continue;
        GuestPtr material = GUEST(GeometricElement, GUEST(GuestPtr, go->elements)[e])->material;
        const GliMaterial *m = GUEST(GliMaterial, material);
        const GliTexture *t = m->texture ? GUEST(GliTexture, m->texture) : NULL;
        Atomic *a = handle_object(OBJECT_ATOMIC, GUEST(u32, d->atomics)[mesh++]);
        if (t && t->ready && (t->flags & 0x128)) {
            u8 mode = t->flags & 8 ? BLEND_TEXTURE : t->flags & 0x100 ? BLEND_MATERIAL : 0;
            if (mode && a) {
                state_mode(mode);
                g_view.textured = true;
                draw_atomic(a, NULL, false);
            }
        } else if (a && g_nqueued < MAX_QUEUED) {
            Queued *q = &g_queued[g_nqueued++];
            q->atomic = a;
            q->frame = a->frame && a->frame->parent ? *a->frame->parent : (Frame){0};
            q->frame.parent = NULL;
        }
    }
}

static void shadow_hierarchy(GuestPtr object) {
    const u32 flags = *GUEST(u32, object + 0x30);
    if (!((flags >> 1) & 1) && *GUEST(s32, object) == 8) {
        GuestPtr geometric = *GUEST(GuestPtr, *GUEST(GuestPtr, object + 4));
        GuestPtr driver = GUEST(GeometricObject, geometric)->driver_object;
        if (driver) {
            Frame *f = handle_object(OBJECT_FRAME, GUEST(DriverObject, driver)->frame);
            if (f) frame_from_matrix(f, GUEST(PosMatrix, *GUEST(GuestPtr, object + 0x24))->m);
            shadow_object(geometric);
        }
    }
    for (GuestPtr child = *GUEST(GuestPtr, object + 8); child; child = *GUEST(GuestPtr, child + 0x14))
        shadow_hierarchy(child);
}

static void place_camera(GuestPtr matrix, int width, int height) {
    GuestPtr local = game_stack_push(0x80);
    inv_POS_fn_vRemoveScaleFromMatrix(local, matrix);
    const float *l = GUEST(float, local);
    for (int k = 0; k < 3; k++) {
        g_view.right[k] = l[0 + k];
        g_view.up[k] = l[4 + k];
        g_view.at[k] = l[8 + k];
    }
    for (int k = 0; k < 3; k++) g_view.pos[k] = fsub(l[12 + k], fmul(g_view.at[k], 10.0f));
    game_stack_pop(0x80);
    const float *m = GUEST(float, matrix);
    g_view.scale[0] = -fsub(fdiv(2.0f, m[0x10]), fdiv(4.0f, (float)height));
    g_view.scale[1] = -fsub(fdiv(2.0f, m[0x11]), fdiv(4.0f, (float)width));
}

void gli_shadow_character(GuestPtr object, GuestPtr matrix, GuestPtr material) {
    const GliTexture *info = material_texture(material);
    if (!info) return;
    u16 width, height;
    memcpy(&width, (const u8 *)info + 8, 2);
    memcpy(&height, (const u8 *)info + 0xA, 2);
    ShadowSize *z = &g_sizes[height == 0x80 ? 0 : 1];
    if (z->next >= z->count) return;
    ShadowCamera *camera = &z->cameras[z->next];

    GsState clear = {.ztst = GS_Z_ALWAYS, .zwrite = true, .colclamp = true, .target = camera->texture.image};
    GsVertex corners[2] = {{.q = 1.0f, .fog = 255}, {.x = (float)z->side, .y = (float)z->side, .q = 1.0f, .fog = 255}};
    render_draw(GS_SPRITE, &clear, corners, 2);

    place_camera(matrix, width, height);
    g_view.near_plane = 0.25f;
    g_view.width = g_view.height = z->side;
    g_view.target = camera->texture.image;
    memcpy(g_view.color, (const u8[4]){0, 0, 0, 255}, 4);

    const bool fog = render_state.fog;
    render_fog(false);
    state_z_write(false);
    state_z_test(false);
    draw_into_shadow(&g_view);
    g_nqueued = 0;
    shadow_hierarchy(object);
    state_mode(BLEND_OPAQUE);
    g_view.textured = false;
    for (int i = 0; i < g_nqueued; i++) draw_atomic(g_queued[i].atomic, &g_queued[i].frame, false);
    draw_into_shadow(NULL);
    state_z_write(true);
    state_z_test(true);
    render_fog(fog);

    GliTexture *out = material_texture(material);
    if (out) out->handle = camera->texture.handle;
    z->next++;
}

typedef struct ShadowMesh {
    GuestPtr vertices;
    u32 count;
    u32 capacity;
    GuestPtr material;
} ShadowMesh;

typedef struct VertexBuffer {
    GuestPtr storage;
    s32 count;
    s32 capacity;
    s32 unk0C;
} VertexBuffer;

static GuestPtr g_mesh;
static float g_origin[3];

s32 gli_shadow_buffer(GuestPtr out, s32 count) {
    *GUEST(GuestPtr, out) = 0;
    GuestPtr array = game_alloc(sizeof(VertexBuffer));
    if (!array) return 0;
    *GUEST(GuestPtr, out) = array;
    VertexBuffer *b = GUEST(VertexBuffer, array);
    s32 capacity = count * 3;
    u32 size = (u32)capacity * 0x24;
    GuestPtr storage = game_alloc(size + 0xF);
    b->storage = storage;
    if (!storage) return 0;
    if (storage & 0xF) b->storage = (storage + 0x10) & ~0xFu;
    memset(GUEST(u8, b->storage), 0, size);
    b->capacity = capacity;
    b->count = 0;
    b->unk0C = 0;
    return 1;
}

void gli_shadow_owner(GuestPtr sound, GuestPtr character) { *GUEST(GuestPtr, sound + 0xC) = character; }

void gli_shadow_mesh(GuestPtr mesh, GuestPtr position) {
    g_mesh = mesh;
    GUEST(ShadowMesh, mesh)->count = 0;
    const float *p = GUEST(float, position);
    for (int k = 0; k < 3; k++) g_origin[k] = fmul(p[k], 0.01f);
}

void gli_shadow_mesh_end(void) { g_mesh = 0; }

void gli_shadow_vertex(GuestPtr position, GuestPtr uv, float alpha) {
    ShadowMesh *m = GUEST(ShadowMesh, g_mesh);
    if (m->count >= m->capacity) return;
    u8 *v = GUEST(u8, m->vertices + m->count * 0x24);
    m->count++;
    const float *p = GUEST(float, position);
    float d[3] = {fsub(p[0], g_origin[0]), fsub(p[1], g_origin[1]), fsub(p[2], g_origin[2])};
    memcpy(v, d, 12);
    v[0xC] = v[0xD] = v[0xE] = 0xFF;
    v[0xF] = (u8)fcvtw(fmul(alpha, 255.0f));
    memcpy(v + 0x1C, GUEST(float, uv), 8);
}

void gli_shadow_object(GuestPtr object, u32 render_flags) {
    GliContext *ctx = gli_context();
    ctx->object = object;
    ctx->render_flags = render_flags & 0xF7FBFBF7u;
    ctx->state_flags = 0x8000;
    ctx->geometric_object = 0;
    ctx->current_matrix = 0;
    ctx->geometry_matrix = 0;
    ctx->camera = 0;
    ctx->material = 0;
    ctx->unk2034 = 0;
    if (*GUEST(u8, DRIVER_BLEND) != BLEND_UNKNOWN) state_mode(BLEND_UNKNOWN);
    gli_context()->draw_flags = 0xFFFFFFFFu;
    state_quads();
}

void gli_shadow_draw(GuestPtr mesh, GuestPtr fog) {
    GliContext *ctx = gli_context();
    const ShadowMesh *m = GUEST(ShadowMesh, mesh);
    ctx->fog_on = fog != 0;
    ctx->fog = fog;
    ctx->material = m->material;
    ctx->texture = GUEST(GliMaterial, m->material)->texture;
    state_forget_addressing();
    state_select_texture();
    static QuadVertex *v;
    static u32 cap;
    if (m->count > cap) {
        cap = m->count;
        v = realloc(v, sizeof *v * cap);
    }
    for (u32 i = 0; i < m->count; i++) {
        const u8 *g = GUEST(u8, m->vertices + i * 0x24);
        memcpy(v[i].pos, g, 12);
        memcpy(v[i].color, g + 0xC, 4);
        memcpy(&v[i].u, g + 0x1C, 4);
        memcpy(&v[i].v, g + 0x20, 4);
    }
    draw_world_triangles(v, (int)m->count);
}
