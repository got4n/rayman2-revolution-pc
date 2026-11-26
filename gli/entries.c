#include "driver.h"

s32 drv_open(u32 a0, u32 a1, GuestPtr init) {
    (void)a0;
    (void)a1;
    return gli_open(init);
}

void drv_close_rasterizer(void) {}

void drv_startup_image(GuestPtr name, s32 width, s32 height, u32 flip) {
    gli_startup_image(name, width, height, (u8)flip);
}
