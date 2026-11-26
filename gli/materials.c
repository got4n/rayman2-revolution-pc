#include <stdlib.h>

#include "gli.h"
#include "handles.h"

typedef struct MaterialColor { float rgba[4]; } MaterialColor;

u32 gli_create_material(void) {
    Material *m = calloc(1, sizeof *m);
    m->color[0] = m->color[1] = m->color[2] = m->color[3] = 0xFF;
    m->handle = handle_new(OBJECT_MATERIAL, m);
    return m->handle;
}

static u8 channel(float c) {
    if (c < 0.0f) return 0;
    if (1.0f < c) return 0xFF;
    return (u8)fcvtw(fmul(c, 255.0f));
}

void gli_material_color(u32 material, GuestPtr color) {
    Material *m = handle_object(OBJECT_MATERIAL, material);
    if (!m) return;
    const MaterialColor *c = GUEST(MaterialColor, color);
    for (int k = 0; k < 4; k++) m->color[k] = channel(c->rgba[k]);
}

u32 gli_material_texture(u32 material, u32 texture) {
    Material *m = handle_object(OBJECT_MATERIAL, material);
    if (m) m->texture = texture;
    return material;
}

void gli_destroy_material(u32 material) {
    Material *m = handle_object(OBJECT_MATERIAL, material);
    if (!m) return;
    handle_free(m->handle);
    free(m);
}
