#ifndef GLI_GLIGLOU_H
#define GLI_GLIGLOU_H

#include "game_vars.h"
#include "guest.h"

enum {
    DRAW_IGNORE_GLOBAL_ALPHA = 0x00000008,
    DRAW_Z_WRITE = 0x08000000,
    DRAW_Z_TEST = 0x40000000,
};

enum {
    STATE_NOT_THIS_PASS = 0x00008000,
};

typedef struct GliContext {
    GuestPtr object;
    GuestPtr geometric_object;
    u32 render_flags;
    u32 draw_flags;
    u32 state_flags;
    u8 unk0014[0x1F50];
    GuestPtr geometry_matrix;
    GuestPtr current_matrix;
    u8 unk1F6C[0xA8];
    float focal, inverse_focal;
    u8 unk201C[0x10];
    GuestPtr texture;
    GuestPtr material;
    s32 unk2034;
    GuestPtr camera;
    s32 unk203C;
    u8 unk2040[0x10];
    u8 vertex_color[4];
    u8 unk2054[0xC];
    float im3d_matrix[4][4];
    GuestPtr fog;
    GuestPtr applied_fog;
    s32 fog_on;
    s32 later_pass;
    s32 pass_flags;
    s32 no_clear;
    s32 unk20B8;
    float global_alpha;
} GliContext;

_Static_assert(offsetof(GliContext, draw_flags) == 0x0C, "GliContext");
_Static_assert(offsetof(GliContext, current_matrix) == 0x1F68 && offsetof(GliContext, focal) == 0x2014, "GliContext");
_Static_assert(offsetof(GliContext, vertex_color) == 0x2050 && offsetof(GliContext, im3d_matrix) == 0x2060 &&
               offsetof(GliContext, unk203C) == 0x203C, "GliContext");
_Static_assert(offsetof(GliContext, texture) == 0x202C && offsetof(GliContext, material) == 0x2030, "GliContext");
_Static_assert(offsetof(GliContext, fog) == 0x20A0 && offsetof(GliContext, global_alpha) == 0x20BC, "GliContext");

#define GLI_CONTEXT GAME_VAR(inv_GLI_renderContext)

static inline GliContext *gli_context(void) { return GUEST(GliContext, *GUEST(GuestPtr, GLI_CONTEXT)); }

enum {
    TEXTURE_NOT_DEFERRED = 0x0008,
    TEXTURE_ADDITIVE = 0x0020,
    TEXTURE_SORTED = 0x0028,
    TEXTURE_NOT_SOLID = 0x0038,
    TEXTURE_BLENDED = 0x0100,
    TEXTURE_UNLIT = 0x0400,
};

typedef struct GliTexture {
    GuestPtr handle;
    u16 flags;
    u8 addressing;
    u8 unk07[9];
    u8 ready;
} GliTexture;

_Static_assert(offsetof(GliTexture, addressing) == 0x06 && offsetof(GliTexture, ready) == 0x10, "GliTexture");

enum {
    MATERIAL_SOLID_PASS = 0x04000000,
};

enum { MATERIAL_SCROLL_ANY = 0x16 };

typedef struct GliMaterial {
    u32 type;
    float color[4];
    u32 frame;
    u8 unk18[0x10];
    GuestPtr texture;
    u8 unk2C[3];
    u8 scroll_flags;
    float scroll_u, scroll_v;
    u8 unk38[0x10];
    u32 driver_material;
} GliMaterial;

_Static_assert(offsetof(GliMaterial, texture) == 0x28 && offsetof(GliMaterial, scroll_u) == 0x30 &&
               offsetof(GliMaterial, driver_material) == 0x48, "GliMaterial");

typedef struct PosMatrix {
    float m[4][4];
} PosMatrix;

typedef struct GliFog {
    float near_distance;
    float far_distance;
    float end_distance;
    float intensity_near;
    float intensity_far;
    float red, green, blue;
} GliFog;

_Static_assert(sizeof(GliFog) == 0x20, "GliFog");

#define DRIVER_BLEND GAME_VAR(inv_GLI_blendModeState)
#define DRIVER_Z_WRITE GAME_VAR(inv_GLI_zWriteState)
#define DRIVER_Z_TEST GAME_VAR(inv_GLI_zTestState)

enum Blend {
    BLEND_UNKNOWN = 0,
    BLEND_UNTEXTURED = 1,
    BLEND_ADD = 2,
    BLEND_TEXTURE = 3,
    BLEND_OPAQUE = 4,
    BLEND_MATERIAL = 5,
};

#define LEVEL_OBJECTS GAME_VAR(inv_SNA_p_hLevelGeometricObjects)
#define LEVEL_MATERIALS GAME_VAR(inv_RW3_p_pLevelMaterials)
#define LEVEL_TEXTURES GAME_VAR(inv_RW3_p_stLevelTextures)
#define LEVEL_TEXTURE_COUNT GAME_VAR(inv_RW3_ulLevelTextureCount)
#define LEVEL_LIGHTS GAME_VAR(inv_RW3_p_pLevelLights)
#define LEVEL_LIGHTMAPS GAME_VAR(inv_RW3_lightmapSlots)
#define LEVEL_LIGHTMAP_COUNT GAME_VAR(inv_RW3_ulLightmapSlotCount)

typedef struct LevelTexture {
    GuestPtr texture;
    s32 number;
} LevelTexture;

typedef struct LevelMaterialLayer {
    GuestPtr texture;
    u8 unk04[3];
    u8 flags;
    u8 unk08[0x18];
    u32 material;
} LevelMaterialLayer;

enum { LAYER_CONTINUES = 0x40 };
enum { LEVEL_MATERIAL_COLOR_FROM_TEXTURE = 0x04000000 };

typedef struct LevelMaterial {
    u32 flags;
    float color[3];
    u8 unk10[0x18];
    LevelMaterialLayer layers[1];
} LevelMaterial;

typedef struct LightmapSlot {
    u32 red, green, blue;
    GuestPtr uvs;
} LightmapSlot;

enum { ELEMENT_MESH = 1 };
enum { OBJECT_FRAME_PER_MESH = 0x08 };

typedef struct GeometricObject {
    GuestPtr element_types;
    GuestPtr elements;
    u32 flags;
    u8 unk0C[0x10];
    GuestPtr driver_object;
    u8 unk20[2];
    s16 element_count;
} GeometricObject;

typedef struct GeometricElement {
    GuestPtr material;
    u8 unk04[2];
    s16 lightmap;
} GeometricElement;

#define MATERIAL_DRIVER_MATERIAL 0x48

typedef struct DriverObject {
    u32 frame;
    GuestPtr atomics;
    u8 unk08[4];
    GuestPtr atomic_states;
    GuestPtr scroll_offsets;
    u16 atomic_count;
    u8 frames;
} DriverObject;

_Static_assert(offsetof(LevelMaterialLayer, material) == 0x20 && sizeof(LevelMaterialLayer) == 0x24 &&
               offsetof(LevelMaterial, layers) == 0x28 && offsetof(GeometricObject, driver_object) == 0x1C &&
               offsetof(GeometricObject, element_count) == 0x22 && offsetof(GeometricElement, lightmap) == 6 &&
               offsetof(DriverObject, atomic_states) == 0x0C && offsetof(DriverObject, frames) == 0x16,
               "level");

enum { LIGHT_DIRECTIONAL = 1, LIGHT_POINT = 2, LIGHT_KIND_3 = 3, LIGHT_AMBIENT = 4, LIGHT_HOTSPOT = 5, LIGHT_FOG = 6 };

typedef struct GliLight {
    u8 enabled;
    u8 unk01;
    u16 kind;
    float far_distance;
    float near_distance;
    float angle0, angle1;
    float tan_half_angle0, tan_half_angle1;
    float matrix[4][4];
    float scale[4];
    u8 matrix_type, pad6D[3];
    float color[4];
    u8 unk80[0x60];
    u32 driver_light;
} GliLight;

_Static_assert(offsetof(GliLight, color) == 0x70 && offsetof(GliLight, driver_light) == 0xE0, "GliLight");

#define GLI_IS_INIT 0x00119178u
#define GLI_SET_LIGHT_SWITCH 0x001199D8u

#endif
