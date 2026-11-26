#include "game_vars.h"
#include <stdlib.h>
#include <string.h>

#include "gli.h"
#include "render.h"

#define OBJECT_MATRIX GAME_VAR(g_p_stCurrentMatrix)
#define SORTED_SPRITES GAME_VAR(inv_SCT_bSendingSectorToViewport)
#define PART_DRAWING GAME_VAR(g_bTakeCareOfAnimationCoordination)
#define PART_SELECTING GAME_VAR(g_bOneShotAnim)
#define PART_DONE GAME_VAR(g_bHasTextureAnimationFinished)
#define PART_COUNT GAME_VAR(g_BaseTimeForParticleAnimation)

enum { RENDER_QUADS = ~0x408 };
enum { ELEMENT_HALO = 0x20 };
enum { MATRIX_IDENTITY = 1 };
enum { POS_MATRIX_TYPE = 0x50 };

enum { VIEW_SPRITES, FLAT_SPRITES, ORIENTED_SPRITES, CORNER_QUADS, ELEMENT_SPRITES, BATCHES };
static const u32 k_capacity[BATCHES] = {400, 300, 350, 250, 400};

static Quad g_quad;
static Quad *g_batch[BATCHES];
static GuestPtr *g_texture[BATCHES];
static u32 g_count[BATCHES];

void sprites_init(void) {
    static const float corners[4][2] = {{1, 0}, {0, 0}, {0, 1}, {1, 1}};
    for (int i = 0; i < 4; i++)
        g_quad.v[i] = (QuadVertex){.color = {255, 255, 255, 255}, .u = corners[i][0], .v = corners[i][1]};
    for (int b = 0; b < BATCHES; b++) {
        if (!g_batch[b]) {
            g_batch[b] = malloc(sizeof *g_batch[b] * k_capacity[b]);
            g_texture[b] = calloc(k_capacity[b], sizeof *g_texture[b]);
        }
        for (u32 i = 0; i < k_capacity[b]; i++) g_batch[b][i] = g_quad;
        g_count[b] = 0;
    }
    queue_init(&g_quad);
}

static void draw_from_camera(GuestPtr viewport, const Quad *quads, int count) {
    const float(*c)[4] = (const float(*)[4])(RAM + gld_camera_matrix(viewport));
    float(*m)[4] = gli_context()->im3d_matrix;
    for (int k = 0; k < 3; k++) {
        m[0][k] = c[k][0];
        m[1][k] = c[k][1];
        m[2][k] = c[k][2];
        m[3][k] = world_origin()[k];
    }
    u32 flags;
    memcpy(&flags, &m[0][3], 4);
    flags &= ~0x20003u;
    memcpy(&m[0][3], &flags, 4);
    static Quad *world;
    static int cap;
    if (count > cap) {
        cap = count;
        world = realloc(world, sizeof *world * (size_t)cap);
    }
    for (int q = 0; q < count; q++) {
        world[q] = quads[q];
        for (int i = 0; i < 4; i++) {
            const float *p = quads[q].v[i].pos;
            for (int k = 0; k < 3; k++)
                world[q].v[i].pos[k] = fadd(fadd(fmul(p[0], m[0][k]), fmul(p[1], m[1][k])), fmul(p[2], m[2][k]));
        }
    }
    draw_quads(world, count);
}

void sprites_draw_quad(GuestPtr viewport, const Quad *quad, GuestPtr texture) {
    GliContext *ctx = gli_context();
    ctx->draw_flags = 0xFFFFFFFFu;
    state_quads();
    ctx->texture = texture;
    state_select_texture();
    draw_from_camera(viewport, quad, 1);
}

static void draw_batch(GuestPtr viewport, int batch, const Quad *quads, int count) {
    if (batch == VIEW_SPRITES || batch == ELEMENT_SPRITES) draw_from_camera(viewport, quads, count);
    else draw_quads(quads, count);
}

void gli_draw_sprites(GuestPtr viewport) {
    GliContext *ctx = gli_context();
    ctx->draw_flags = 0xFFFFFFFFu;
    state_quads();
    for (int b = 0; b < BATCHES; b++) {
        u32 done = 0;
        while (done < g_count[b]) {
            u32 run = 1;
            while (done + run < g_count[b] && g_texture[b][done + run] == g_texture[b][done]) run++;
            ctx = gli_context();
            ctx->texture = g_texture[b][done];
            state_select_texture();
            draw_batch(viewport, b, &g_batch[b][done], (int)run);
            done += run;
        }
    }
    memset(g_count, 0, sizeof g_count);
}

static void set_color(Quad *q, const u8 vertex_color[4]) {
    for (int i = 0; i < 4; i++) {
        q->v[i].color[0] = vertex_color[2];
        q->v[i].color[1] = vertex_color[1];
        q->v[i].color[2] = vertex_color[0];
        q->v[i].color[3] = vertex_color[3];
    }
}

static void facing_corners(Quad *q, const float c[3], float hw, float hh) {
    float left = fsub(c[0], hw), bottom = fsub(c[1], hh), right = fadd(c[0], hw), top = fadd(c[1], hh);
    const float xy[4][2] = {{left, bottom}, {right, bottom}, {right, top}, {left, top}};
    for (int i = 0; i < 4; i++) {
        q->v[i].pos[0] = xy[i][0];
        q->v[i].pos[1] = xy[i][1];
        q->v[i].pos[2] = c[2];
    }
}

static void flat_corners(Quad *q, const float c[3], float hw, float hh) {
    float left = fsub(c[0], hw), bottom = fsub(c[1], hh), right = fadd(c[0], hw), top = fadd(c[1], hh);
    const float xy[4][2] = {{left, bottom}, {left, top}, {right, top}, {right, bottom}};
    for (int i = 0; i < 4; i++) {
        q->v[i].pos[0] = xy[i][0];
        q->v[i].pos[1] = xy[i][1];
        q->v[i].pos[2] = c[2];
    }
}

static void oriented_corners(Quad *q, const float c[3], const float a0[3], const float a1[3], float s0, float s1) {
    for (int k = 0; k < 3; k++) {
        float x0 = fmul(s0, a0[k]), x1 = fmul(s1, a1[k]);
        float difference = fsub(x0, x1), sum = fadd(x0, x1);
        q->v[0].pos[k] = fsub(c[k], difference);
        q->v[1].pos[k] = fadd(c[k], sum);
        q->v[2].pos[k] = fadd(c[k], difference);
        q->v[3].pos[k] = fsub(c[k], sum);
    }
}

static Quad *batch_quad(int batch, GuestPtr texture) {
    if (g_count[batch] == k_capacity[batch]) return NULL;
    g_texture[batch][g_count[batch]] = texture;
    return &g_batch[batch][g_count[batch]++];
}

static bool put_off(void) { return gli_context()->state_flags & STATE_NOT_THIS_PASS; }

static void select_material(void) { GLI_vDoMaterialSelection(); }

static void point_through(GuestPtr matrix, const float in[3], float out[3],
                          void (*function)(GuestPtr, GuestPtr, GuestPtr)) {
    GuestPtr frame = game_stack_push(32);
    memcpy(GUEST(float, frame + 16), in, 12);
    function(frame, matrix, frame + 16);
    memcpy(out, GUEST(float, frame), 12);
    game_stack_pop(32);
}

static float x_scale(GuestPtr matrix) { return POS_fn_xGetXScale(matrix); }

typedef struct SpriteElement {
    u16 flags;
    s16 vertex;
    float half_width, half_height;
    GuestPtr material;
} SpriteElement;

typedef struct SpriteList {
    GuestPtr sprites;
    s16 count;
} SpriteList;

typedef struct SpriteObject {
    u8 unk00[8];
    u32 flags;
    u8 unk0C[0x18];
    GuestPtr vertices;
} SpriteObject;

_Static_assert(sizeof(SpriteElement) == 0x10 && offsetof(SpriteObject, vertices) == 0x24, "sprites");

static void element_center(const SpriteElement *e, GuestPtr vertices, float c[3]) {
    GuestPtr matrix = *GUEST(GuestPtr, OBJECT_MATRIX);
    const float *v = GUEST(float, vertices + (u32)e->vertex * 12);
    if (*GUEST(u8, matrix + POS_MATRIX_TYPE) != MATRIX_IDENTITY) point_through(matrix, v, c, POS_fn_vMulMatrixVertex);
    else memcpy(c, v, 12);
    point_through(gld_camera_matrix(gli_context()->object), c, c, POS_fn_vApplyMatrixToVector);
}

static void lit_color(const GliMaterial *m, bool lit) {
    float ambient[3] = {1.0f, 0.0f, 0.0f}, parallel[3] = {0.0f, 0.0f, 0.0f};
    if (lit) {
        bool have_ambient = false, have_parallel = false;
        for (s32 j = 0; j < lights_viewport_count() && (!have_ambient || !have_parallel); j++) {
            if (!((lights_mask() >> j) & 1)) continue;
            const GliLight *l = GUEST(GliLight, lights_viewport(j));
            if (l->kind == LIGHT_AMBIENT) {
                memcpy(ambient, l->color, 12);
                have_ambient = true;
            } else if (l->kind == LIGHT_DIRECTIONAL) {
                memcpy(parallel, l->color, 12);
                have_parallel = true;
            }
        }
    }
    s32 rgb[3];
    for (int k = 0; k < 3; k++) {
        float c = fmul(m->color[k], 255.0f);
        if (lit) c = fmul(c, fadd(ambient[k], fmul(parallel[k], 0.5f)));
        rgb[k] = fcvtw(c < 255.0f ? c : 255.0f);
    }
    GliContext *ctx = gli_context();
    u32 packed;
    memcpy(&packed, ctx->vertex_color, 4);
    packed = (packed & 0xFF000000u) | (u32)(rgb[0] << 16 | rgb[1] << 8 | rgb[2]);
    memcpy(ctx->vertex_color, &packed, 4);
}

void gli_sprite_elements(GuestPtr viewport, GuestPtr list_address) {
    (void)viewport;
    const SpriteList *list = GUEST(SpriteList, list_address);
    const SpriteElement *sprites = GUEST(SpriteElement, list->sprites);
    if (sprites[0].flags & ELEMENT_HALO) {
        for (int i = 0; i < list->count; i++)
            halos_record(gli_context()->geometric_object, list->sprites + (u32)i * (u32)sizeof(SpriteElement));
        return;
    }
    GliContext *ctx = gli_context();
    u32 saved = ctx->render_flags;
    ctx->render_flags = saved & ~0x400u;
    ctx->material = sprites[0].material;
    select_material();
    ctx = gli_context();
    const GuestPtr vertices = GUEST(SpriteObject, ctx->geometric_object)->vertices;
    const GuestPtr matrix = *GUEST(GuestPtr, OBJECT_MATRIX);
    const float scale = x_scale(matrix);
    if (put_off()) {
        const GliMaterial *m = GUEST(GliMaterial, ctx->material);
        if (*GUEST(s8, SORTED_SPRITES) == 1) {
            if (GUEST(SpriteObject, ctx->geometric_object)->flags & OBJECT_FRAME_PER_MESH) {
                const GliTexture *t = m->texture ? GUEST(GliTexture, m->texture) : NULL;
                if (t && !(t->flags & TEXTURE_ADDITIVE)) lit_color(m, true);
            } else {
                lit_color(m, false);
            }
            for (int i = 0; i < list->count; i++) {
                Quad *q = queue_quad(GUEST(GliMaterial, gli_context()->material)->texture);
                if (!q) continue;
                float c[3];
                element_center(&sprites[i], vertices, c);
                facing_corners(q, c, fmul(scale, sprites[i].half_width), fmul(scale, sprites[i].half_height));
                set_color(q, gli_context()->vertex_color);
            }
        } else {
            for (int i = 0; i < list->count; i++) {
                Quad *q = batch_quad(ELEMENT_SPRITES, GUEST(GliMaterial, gli_context()->material)->texture);
                if (!q) continue;
                float c[3];
                element_center(&sprites[i], vertices, c);
                facing_corners(q, c, fmul(scale, sprites[i].half_width), fmul(scale, sprites[i].half_height));
                set_color(q, gli_context()->vertex_color);
            }
        }
    } else {
        for (int i = 0; i < list->count; i++) {
            float c[3];
            element_center(&sprites[i], vertices, c);
            facing_corners(&g_quad, c, fmul(scale, sprites[i].half_width), fmul(scale, sprites[i].half_height));
            set_color(&g_quad, gli_context()->vertex_color);
            draw_from_camera(gli_context()->object, &g_quad, 1);
        }
    }
    gli_context()->render_flags = saved;
}

static void part_begin(float count) {
    *GUEST(u8, PART_DRAWING) = 1;
    *GUEST(float, PART_COUNT) = count;
    *GUEST(u8, PART_SELECTING) = 1;
    *GUEST(u8, PART_DONE) = 0;
}

static u8 part_end(u32 saved_flags, bool restore) {
    if (restore) {
        gli_context()->unk203C = 0;
        gli_context()->render_flags = saved_flags;
    }
    *GUEST(u8, PART_DRAWING) = 0;
    *GUEST(u8, PART_SELECTING) = 0;
    return *GUEST(u8, PART_DONE);
}

static GuestPtr material_texture(GuestPtr material) { return GUEST(GliMaterial, material)->texture; }

u8 gli_part_facing(GuestPtr center, GuestPtr material, GuestPtr viewport, u32 count, float hw, float hh) {
    GliContext *ctx = gli_context();
    u32 saved = ctx->render_flags;
    part_begin((float)count);
    ctx->render_flags = (u32)RENDER_QUADS;
    ctx->material = material;
    select_material();
    ctx = gli_context();
    ctx->later_pass = 1;
    const GuestPtr camera_address = gld_camera_matrix(viewport);
    float c[3];
    if (put_off()) {
        Quad *q = batch_quad(VIEW_SPRITES, material_texture(material));
        if (q) {
            point_through(camera_address, GUEST(float, center), c, POS_fn_vApplyMatrixToVector);
            facing_corners(q, c, hw, hh);
            set_color(q, ctx->vertex_color);
        }
    } else {
        point_through(camera_address, GUEST(float, center), c, POS_fn_vApplyMatrixToVector);
        facing_corners(&g_quad, c, hw, hh);
        set_color(&g_quad, ctx->vertex_color);
        draw_from_camera(viewport, &g_quad, 1);
    }
    return part_end(saved, true);
}

u8 gli_part_flat(GuestPtr center, GuestPtr material, GuestPtr viewport, u32 count, float hw, float hh) {
    (void)viewport;
    GliContext *ctx = gli_context();
    u32 saved = ctx->render_flags;
    part_begin((float)count);
    u32 flags = saved & (u32)RENDER_QUADS;
    ctx->material = material;
    ctx->render_flags = flags;
    ctx->draw_flags = flags;
    ctx->later_pass = 1;
    ctx->unk203C = 1;
    select_material();
    ctx = gli_context();
    const float *c = GUEST(float, center);
    if (put_off()) {
        Quad *q = batch_quad(FLAT_SPRITES, material_texture(material));
        if (q) {
            flat_corners(q, c, hw, hh);
            set_color(q, ctx->vertex_color);
        }
    } else {
        flat_corners(&g_quad, c, hw, hh);
        set_color(&g_quad, ctx->vertex_color);
        draw_quads(&g_quad, 1);
    }
    return part_end(saved, true);
}

u8 gli_part_oriented(float s0, float s1, GuestPtr center, GuestPtr material, GuestPtr viewport, u32 count,
                     GuestPtr axis0, GuestPtr axis1) {
    (void)viewport;
    GliContext *ctx = gli_context();
    part_begin((float)count);
    u32 saved = ctx->render_flags;
    ctx->material = material;
    u32 flags = saved & (u32)RENDER_QUADS;
    ctx->render_flags = flags;
    ctx->draw_flags = flags;
    select_material();
    ctx = gli_context();
    ctx->later_pass = 1;
    ctx->unk203C = 1;
    const float *c = GUEST(float, center), *a0 = GUEST(float, axis0), *a1 = GUEST(float, axis1);
    if (put_off()) {
        Quad *q = batch_quad(ORIENTED_SPRITES, material_texture(material));
        if (q) {
            oriented_corners(q, c, a0, a1, s0, s1);
            set_color(q, ctx->vertex_color);
        }
    } else {
        oriented_corners(&g_quad, c, a0, a1, s0, s1);
        set_color(&g_quad, ctx->vertex_color);
        draw_quads(&g_quad, 1);
    }
    return part_end(saved, true);
}

void gli_part_quad(GuestPtr c0, GuestPtr c1, GuestPtr c2, GuestPtr c3, GuestPtr material) {
    GliContext *ctx = gli_context();
    part_begin(1.0f);
    ctx->material = material;
    ctx->render_flags = (u32)RENDER_QUADS;
    select_material();
    ctx = gli_context();
    const GuestPtr corners[4] = {c0, c1, c2, c3};
    Quad *q = put_off() ? batch_quad(CORNER_QUADS, material_texture(material)) : &g_quad;
    if (q) {
        for (int i = 0; i < 4; i++) memcpy(q->v[i].pos, GUEST(float, corners[i]), 12);
        set_color(q, ctx->vertex_color);
        if (q == &g_quad) draw_quads(&g_quad, 1);
    }
    part_end(0, false);
}
