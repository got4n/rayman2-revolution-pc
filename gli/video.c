#include "game_vars.h"
#include <stdlib.h>
#include <string.h>

#include "gli.h"
#include "render.h"

#define DISPLAY_HEIGHT GAME_VAR(g_lVIDEO_WantedHeight)
#define VIDEO_POINT_ON GAME_VAR(inv_FMN_bCdReadRetrying)
#define VIDEO_X GAME_VAR(inv_MPEG_pictureX)
#define VIDEO_Y GAME_VAR(inv_MPEG_pictureY)
#define VIDEO_BUFFER GAME_VAR(inv_MPEG_displayBuffer)
#define VIDEO_PICTURE GAME_VAR(inv_MPEG_pictureBuffer)
#define VIDEO_SIZE GAME_VAR(inv_MPEG_decoder)
#define POINT_ON GAME_VAR(inv_MPEG_retryIconOn)
#define POINT_OFF GAME_VAR(inv_MPEG_retryIconOff)

void gli_fade_copy(void) { render_copy_shown(); }

static int power_of_two(int n) {
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

static void copy_image(GuestPtr texels, int x, int y, int width, int height) {
    if (width <= 0 || height <= 0 || width > 1024 || height > 1024) return;
    const int tw = power_of_two(width), th = power_of_two(height);
    u32 *t = calloc((size_t)tw * (size_t)th, 4);
    for (int row = 0; row < height; row++)
        memcpy(t + (size_t)row * (size_t)tw, GUEST(u32, texels) + (size_t)row * (size_t)width, (size_t)width * 4);
    GsState s = {.tme = true, .tfx = GS_DECAL, .tcc = true, .wrap_s = GS_CLAMP, .wrap_t = GS_CLAMP,
                 .ztst = GS_Z_ALWAYS, .colclamp = true};
    s.texture = render_texture_new(tw, th, t);
    free(t);
    GsVertex v[2] = {
        {.x = (float)x, .y = (float)y, .q = 1.0f, .fog = 255},
        {.x = (float)(x + width), .y = (float)(y + height), .s = (float)width / (float)tw,
         .t = (float)height / (float)th, .q = 1.0f, .r = 0x80, .g = 0x80, .b = 0x80, .a = 0x80, .fog = 255},
    };
    render_2d_widescreen_centre(v, 2);
    render_draw(GS_SPRITE, &s, v, 2);
    render_texture_free(s.texture);
}

int revo_loading_movie_hidden(void);

static void black(int height) {
    GsState s = {.ztst = GS_Z_ALWAYS, .colclamp = true};
    GsVertex v[2] = {{.x = -GS_WIDTH, .y = -GS_HEIGHT, .q = 1.0f, .fog = 255},
                     {.x = 2.0f * GS_WIDTH, .y = 2.0f * (float)height, .q = 1.0f, .a = 0x80, .fog = 255}};
    render_draw(GS_SPRITE, &s, v, 2);
}

void gli_video_frame(void) {
    const s32 height = *GUEST(s32, DISPLAY_HEIGHT);
    if (revo_loading_movie_hidden()) {
        black(height);
        render_show_frame();
        s32 *buffer = GUEST(s32, VIDEO_BUFFER);
        *buffer = 1 - *buffer;
        return;
    }
    copy_image(*GUEST(s8, VIDEO_POINT_ON) ? POINT_ON : POINT_OFF, 0x32, height - 0x42, 16, 16);
    const s32 *size = GUEST(s32, VIDEO_SIZE);
    copy_image(VIDEO_PICTURE, fcvtw(*GUEST(float, VIDEO_X)), fcvtw(*GUEST(float, VIDEO_Y)), size[0], size[1]);
    render_show_frame();
    s32 *buffer = GUEST(s32, VIDEO_BUFFER);
    *buffer = 1 - *buffer;
}
