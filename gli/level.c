#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clump.h"
#include "files.h"
#include "gli.h"
#include "handles.h"
#include "rwstream.h"

static const CallerFrame FRAME = {0xD0, 0x10};

static Clump *g_clump;
static Clump *g_previous;
static int g_next_atomic;
static bool g_created;

const Clump *level_clump(void) { return g_clump; }

s32 gli_load_level_clump(GuestPtr level) {
    lower_in_place(level);
    const char *name = GUEST(char, level);
    char path[0x80];
    snprintf(path, sizeof path, "%s/%s.ato.%d", name, name, 0);
    u32 file = file_open(path, FRAME);
    if (!file_opened(file)) return 1;
    u32 length;
    u8 *bytes = file_read(file, &length);
    Reader stream = {bytes, 0, bytes ? length : 0}, body;
    if (!find_chunk(&stream, CHUNK_CLUMP, &body)) {
        free(bytes);
        return 0;
    }
    if (g_created) {
        clump_free(g_previous);
        g_previous = g_clump;
    } else {
        clump_free(g_clump);
    }
    g_clump = clump_read(bytes, length);
    g_next_atomic = g_clump ? g_clump->atomic_count - 1 : -1;
    free(bytes);
    file_read_done();
    return 1;
}

#define MAX_OBJECTS 0x4000

static Frame *g_frames[MAX_OBJECTS];
static int g_frame_count;
static Atomic *g_atomics[MAX_OBJECTS];
static int g_atomic_count;
static Light *g_lights[MAX_OBJECTS];
static int g_light_count;
static Light *g_ambient;

Light *const *level_lights(int *count) {
    *count = g_light_count;
    return g_lights;
}

Light *level_ambient(void) { return g_ambient; }

ClumpGeometry *level_geometry(const Atomic *a) { return (ClumpGeometry *)a->geometry; }

Atomic *const *level_atomics(int *count) {
    *count = g_atomic_count;
    return g_atomics;
}

void frame_from_matrix(Frame *f, const float m[4][4]) {
    memcpy(f->right, m[0], sizeof f->right);
    memcpy(f->up, m[1], sizeof f->up);
    memcpy(f->at, m[2], sizeof f->at);
    memcpy(f->pos, m[3], sizeof f->pos);
}

static void frame_free(Frame *f) {
    handle_free(f->handle);
    free(f);
}

static Frame *frame_new(Frame *parent) {
    Frame *f = calloc(1, sizeof *f);
    f->right[0] = f->up[1] = f->at[2] = 1.0f;
    f->parent = parent;
    f->handle = handle_new(OBJECT_FRAME, f);
    if (g_frame_count < MAX_OBJECTS) g_frames[g_frame_count++] = f;
    return f;
}

static Atomic *atomic_take(void) {
    if (!g_clump || g_next_atomic < 0) {
        revo_log("[gli] the level clump has no atomic left");
        return NULL;
    }
    const ClumpAtomic *from = &g_clump->atomics[g_next_atomic--];
    Atomic *a = calloc(1, sizeof *a);
    a->geometry = &g_clump->geometries[from->geometry];
    a->flags = (u8)from->flags;
    a->lightmap = -1;
    a->handle = handle_new(OBJECT_ATOMIC, a);
    handle_view(a->handle)[ATOMIC_VIEW_GEOMETRY / 4] = geometry_handle(level_geometry(a));
    if (g_atomic_count < MAX_OBJECTS) g_atomics[g_atomic_count++] = a;
    return a;
}

static bool bind_lightmaps(void) {
    u32 count = *GUEST(u32, LEVEL_LIGHTMAP_COUNT);
    LightmapSlot *slots = GUEST(LightmapSlot, *GUEST(GuestPtr, LEVEL_LIGHTMAPS));
    for (u32 i = 0; i < count; i++) {
        u32 rasters[3];
        for (int layer = 0; layer < 3; layer++) {
            char name[16];
            snprintf(name, sizeof name, "%03u.%d", i, layer);
            Texture *t = textures_take(name);
            if (!t) return false;
            rasters[layer] = t->raster;
        }
        slots[i].red = rasters[0];
        slots[i].green = rasters[1];
        slots[i].blue = rasters[2];
    }
    return true;
}

static void bind_textures(void) {
    u32 count = *GUEST(u32, LEVEL_TEXTURE_COUNT);
    const LevelTexture *entries = GUEST(LevelTexture, *GUEST(GuestPtr, LEVEL_TEXTURES));
    for (u32 i = 0; i < count; i++) {
        if (!entries[i].texture) continue;
        char name[16];
        snprintf(name, sizeof name, "%03d", entries[i].number);
        GliTexture *gt = GUEST(GliTexture, entries[i].texture);
        Texture *t = textures_find(name);
        gt->handle = t ? t->handle : 0;
        if (!t) continue;
        gt->ready = 1;
        t->address_u = gt->addressing & 2 ? ADDRESS_WRAP : ADDRESS_CLAMP;
        t->address_v = gt->addressing & 1 ? ADDRESS_WRAP : ADDRESS_CLAMP;
    }
}

static u8 color_channel(float c) {
    if (c < 0.0f) return 0;
    if (1.0f < c) return 0xFF;
    return (u8)fcvtw(fmul(c, 255.0f));
}

static void create_materials(void) {
    for (GuestPtr *list = GUEST(GuestPtr, *GUEST(GuestPtr, LEVEL_MATERIALS)); *list; list++) {
        LevelMaterial *lm = GUEST(LevelMaterial, *list);
        LevelMaterialLayer *layer = lm->layers;
        for (;; layer++) {
            u32 handle = gli_create_material();
            if (!(lm->flags & LEVEL_MATERIAL_COLOR_FROM_TEXTURE)) {
                Material *m = handle_object(OBJECT_MATERIAL, handle);
                for (int k = 0; k < 3; k++) m->color[k] = color_channel(lm->color[k]);
                m->color[3] = 0xFE;
            } else {
                gli_material_color(handle, *list + (u32)offsetof(LevelMaterial, color));
            }
            if (layer->texture) gli_material_texture(handle, GUEST(GliTexture, layer->texture)->handle);
            layer->material = handle;
            if (!(layer->flags & LAYER_CONTINUES)) break;
        }
    }
}

static void create_object(GuestPtr object) {
    const GeometricObject *go = GUEST(GeometricObject, object);
    if (!go->driver_object) return;
    DriverObject *d = GUEST(DriverObject, go->driver_object);
    Frame *frame = frame_new(NULL);
    d->frame = frame->handle;
    bool frame_per_mesh = go->flags & OBJECT_FRAME_PER_MESH;
    d->frames = frame_per_mesh ? 2 : 1;
    const LightmapSlot *slots = GUEST(LightmapSlot, *GUEST(GuestPtr, LEVEL_LIGHTMAPS));
    int n = 0;
    for (int e = 0; e < go->element_count; e++) {
        if (GUEST(s16, go->element_types)[e] != ELEMENT_MESH) continue;
        Atomic *a = atomic_take();
        if (!a) continue;
        const GeometricElement *element = GUEST(GeometricElement, GUEST(GuestPtr, go->elements)[e]);
        a->material = *GUEST(u32, element->material + MATERIAL_DRIVER_MATERIAL);
        if (!frame_per_mesh) {
            a->frame = frame;
            if (element->lightmap != -1 && slots[element->lightmap].red) a->lightmap = element->lightmap;
        } else {
            a->frame = frame_new(frame);
        }
        GUEST(u32, d->atomics)[n] = a->handle;
        GUEST(u8, d->atomic_states)[n] = 0;
        n++;
    }
}

static void create_light(GuestPtr light) {
    GliLight *gl = GUEST(GliLight, light);
    gl->driver_light = 0;
    u8 type;
    switch (gl->kind) {
    case LIGHT_DIRECTIONAL: type = RW_LIGHT_DIRECTIONAL; break;
    case LIGHT_POINT: type = RW_LIGHT_POINT; break;
    case LIGHT_AMBIENT:
        type = RW_LIGHT_AMBIENT;
        if (g_ambient) {
            gl->driver_light = g_ambient->handle;
            return;
        }
        break;
    case LIGHT_FOG: {
        GLI_vSetLightSwitch(light, 0);
        return;
    }
    default: return;
    }
    Light *l = calloc(1, sizeof *l);
    l->type = type;
    memcpy(l->color, gl->color, sizeof l->color);
    l->handle = handle_new(OBJECT_LIGHT, l);
    gl->driver_light = l->handle;
    if (type == RW_LIGHT_AMBIENT) {
        g_ambient = l;
    } else {
        l->radius = gl->far_distance;
        l->cone_angle = gl->angle1;
        l->frame = frame_new(NULL);
        frame_from_matrix(l->frame, gl->matrix);
    }
    if (g_light_count < MAX_OBJECTS) g_lights[g_light_count++] = l;
}

void gli_create_level_objects(void) {
    if (!bind_lightmaps()) revo_log("[gli] the level's lightmaps are not all in its dictionaries");
    bind_textures();
    icons_forget();
    create_materials();
    for (GuestPtr *list = GUEST(GuestPtr, *GUEST(GuestPtr, LEVEL_OBJECTS)); *list; list++) create_object(*list);
    GuestPtr lights = *GUEST(GuestPtr, LEVEL_LIGHTS);
    if (lights)
        for (GuestPtr *list = GUEST(GuestPtr, lights); *list; list++) create_light(*list);
    g_created = true;
}

void gli_destroy_level_objects(void) {
    if (!g_created) return;
    for (int i = 0; i < g_light_count; i++) {
        handle_free(g_lights[i]->handle);
        free(g_lights[i]);
    }
    g_light_count = 0;
    g_ambient = NULL;
    for (int i = 0; i < g_atomic_count; i++) {
        handle_free(g_atomics[i]->handle);
        free(g_atomics[i]);
    }
    g_atomic_count = 0;
    morphs_clear();
    for (int i = 0; i < g_frame_count; i++) frame_free(g_frames[i]);
    g_frame_count = 0;
    clump_free(g_clump);
    g_clump = NULL;
    clump_free(g_previous);
    g_previous = NULL;
    for (GuestPtr *list = GUEST(GuestPtr, *GUEST(GuestPtr, LEVEL_MATERIALS)); *list; list++)
        for (LevelMaterialLayer *layer = GUEST(LevelMaterial, *list)->layers;; layer++) {
            gli_destroy_material(layer->material);
            if (!(layer->flags & LAYER_CONTINUES)) break;
        }
    textures_purge();
    g_created = false;
}
