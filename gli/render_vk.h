#pragma once

#include <stdbool.h>

#include "render.h"

bool vk_open(int pw, int ph);
bool vk_set_size(int pw, int ph);
int vk_draw_frame(const GsFrame *f);
void vk_release_texture(GsTexture *t);

typedef bool (*VkSurfaceMaker)(void *ctx, void *instance, void *surface);
const char *vk_library(void);
bool vk_open_window(const char *const *extensions, unsigned n, VkSurfaceMaker make, void *ctx);
void vk_show(int b, int x, int y, int w, int h, int mode, int post);
void vk_overlay(const u8 *rgba, int w, int h, int x, int y, int zoom);
void vk_screenshot(const char *name);
void vk_present(int width, int height);

typedef struct VkUi {
    void *instance, *gpu, *device, *queue, *render_pass;
    unsigned family, images;
    void *(*proc)(const char *name);
} VkUi;
bool vk_ui(VkUi *ui);
void vk_ui_hook(void (*record)(void *cmd));
