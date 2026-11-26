#include "game_vars.h"
#include "gli.h"
#include "render.h"

#define WORK_HEAP GAME_VAR(inv_RW3_p_stWorldHeap)
#define CURRENT_HEAP GAME_VAR(gp_6F74)

void gli_create_default_heap(u32 size) { inv_MEM_fn_vInitDefaultHeap(size); }

void gli_create_work_heap(u32 size) {
    u32 heap = inv_MEM_fn_p_vAllocAligned(0, size, 4, 5);
    *GUEST(u32, WORK_HEAP) = heap;
    *GUEST(u32, CURRENT_HEAP) = heap;
}

s32 gli_open(GuestPtr init) {
    if (init && !game_fn_u32(init)) return 0;
    return 1;
}

#define TEXTURE_OPTIONS GAME_VAR(inv_GLI_stDisplayState)

typedef struct TextureOptions {
    u32 unk00;
    u32 unk04;
    u32 unk08;
    u32 unk0C, unk10;
    u32 unk14;
    u32 unk18;
    u32 unk1C;
    u32 unk20;
    u8 unk24;
    u8 pad25[3];
    float unk28;
} TextureOptions;

_Static_assert(sizeof(TextureOptions) == 0x2C, "TextureOptions");

static void no_texture_compression(void) {
    TextureOptions *o = GUEST(TextureOptions, TEXTURE_OPTIONS);
    memset(o, 0, sizeof *o);
    o->unk04 = 0x11;
    o->unk08 = 7;
    o->unk14 = 0x100;
    o->unk18 = 0xFFFFFFFFu;
    o->unk24 = 3;
    o->unk28 = 60.0f;
}

void gli_open_rasterizer(void) {
    cameras_create();
    screen_init();
    sprites_init();
    for (int i = 0; i < 2; i++) {
        render_clear_color(0xFF000000u);
        render_show_frame();
    }
    world_create();
    no_texture_compression();
}
