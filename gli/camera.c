#include "game_vars.h"
#include <math.h>
#include <stdlib.h>

#include "gli.h"
#include "handles.h"
#include "render.h"
#include "mods.h"

#define CURRENT_CAMERA GAME_VAR(inv_GLI_currentCamera)
#define FIXED_VIEWPORT GAME_VAR(dword_252570)
#define FIXED_CAMERA GAME_VAR(dword_2525E8)

static Camera g_main, g_second;
static Camera *g_current;

Camera *camera_current(void) { return g_current; }
Camera *camera_second(void) { return &g_second; }
float cameras_main_near(void) { return g_main.near_plane; }

float cameras_near_screen_z(void) { return 65535.0f; }

#define FAR_SCREEN_Z 0.0f

void camera_depth_terms(const Camera *camera, float *scale_out, float *offset_out) {
    const float epsilon = 9.999999747378752e-05f;
    /* The main camera's depth reaches to infinity: past a far plane, Vulkan's depth clamp gives
       every surface the same depth, and the scenery that hides distant objects on the PS2 (the
       sea behind the intro's rock arch) is drawn under them by order instead. Depth is a 32-bit
       float proportional to 1/z, so near precision does not suffer. */
    float far_q = camera == &g_main ? 0.0f : 1.0f / camera->far_plane, near_q = 1.0f / camera->near_plane;
    float z_near = cameras_near_screen_z(), z_far = FAR_SCREEN_Z;
    float adj_far = z_far + (z_near - z_far) * epsilon;
    float adj_near = z_near + (adj_far - z_near) * epsilon;
    float scale = (adj_far - adj_near) / (far_q - near_q);
    *scale_out = scale;
    *offset_out = adj_far - scale * far_q;
}

float camera_screen_z(const Camera *camera, float q) {
    float scale, offset;
    camera_depth_terms(camera, &scale, &offset);
    return scale * q + offset;
}

static void make_current(Camera *camera) {
    g_current = camera;
    *GUEST(u32, CURRENT_CAMERA) = camera->handle;
}

static void camera_init(Camera *camera, float view_width, float view_height) {
    camera->handle = handle_new(OBJECT_CAMERA, camera);
    camera->view_window[0] = view_width;
    camera->view_window[1] = view_height;
    camera->near_plane = 0.25f;
    camera->far_plane = 500.0f;
}

void cameras_create(void) {
    camera_init(&g_main, 0.7f, fdiv(0.7f, 1.3333334f));
    camera_init(&g_second, 0.7f, 0.525f);
    make_current(&g_main);
}

typedef struct GldViewport {
    u8 unk00[0x68];
    GuestPtr camera_cell;
} GldViewport;

typedef struct GldCamera {
    u32 unk00;
    float matrix[4][4];
} GldCamera;

_Static_assert(offsetof(GldViewport, camera_cell) == 0x68, "GldViewport");
_Static_assert(offsetof(GldCamera, matrix) == 0x04, "GldCamera");

GuestPtr gld_camera_matrix(GuestPtr viewport) {
    GuestPtr cell = GUEST(GldViewport, viewport)->camera_cell;
    return *GUEST(GuestPtr, cell) + (u32)offsetof(GldCamera, matrix);
}

static void rewiden(Camera *c, GuestPtr camera);

void gli_viewport_camera(GuestPtr viewport) {
    if (!*GUEST(u32, CURRENT_CAMERA)) return;
    make_current(viewport == FIXED_VIEWPORT ? &g_second : &g_main);
    GuestPtr cell = GUEST(GldViewport, viewport)->camera_cell;
    GuestPtr gld_camera = *GUEST(GuestPtr, cell);
    rewiden(g_current, gld_camera);
    memcpy(g_current->view, GUEST(GldCamera, gld_camera)->matrix, sizeof g_current->view);
}

typedef struct GliCamera {
    u8 unk00[0x58];
    float fov[2];
    u8 unk60[8];
    float unk68, unk6C, unk70;
    u8 unk74[0x10];
    float focal[2];
    float clip_planes[4][4];
} GliCamera;

_Static_assert(offsetof(GliCamera, fov) == 0x58 && offsetof(GliCamera, unk68) == 0x68 &&
                   offsetof(GliCamera, focal) == 0x84 && offsetof(GliCamera, clip_planes) == 0x8C,
               "GliCamera");

static void widen(GliCamera *g, float across, float down) {
    g->focal[0] /= across;
    g->unk68 /= across;
    g->unk70 *= across;
    g->focal[1] /= down;
    g->unk70 /= down;
    for (int p = 0; p < 4; p++) {
        const int a = p < 2 ? 0 : 1;
        float x = g->clip_planes[p][a], z = g->clip_planes[p][2] * (p < 2 ? across : down), n = sqrtf(x * x + z * z);
        if (n > 0.0f) {
            g->clip_planes[p][a] = x / n;
            g->clip_planes[p][2] = z / n;
        }
    }
}

static void factors(const Camera *c, float f[2]) {
    const float fov = c == &g_main ? (float)revo_set(REVO_SET_FOV) / 100.0f : 1.0f;
    f[0] = widescreen_factor() * fov;
    f[1] = fov;
}

void gli_camera_fov(GuestPtr camera) {
    Camera *c = camera == *GUEST(GuestPtr, FIXED_CAMERA) ? &g_second : &g_main;
    if (!c->handle) return;
    GliCamera *gc = GUEST(GliCamera, camera);
    float f[2];
    factors(c, f);
    c->view_window[0] = tanf(fmul(gc->fov[0], 0.5f));
    c->view_window[1] = tanf(fmul(gc->fov[1], 0.5f));
    if (f[0] != 1.0f || f[1] != 1.0f) {
        c->view_window[0] *= f[0];
        c->view_window[1] *= f[1];
        widen(gc, f[0], f[1]);
    }
    c->gli = camera;
    c->wide[0] = f[0];
    c->wide[1] = f[1];
}

static void rewiden(Camera *c, GuestPtr camera) {
    if (!c->wide[0] || c->gli != camera) return;
    float f[2];
    factors(c, f);
    if (f[0] == c->wide[0] && f[1] == c->wide[1]) return;
    const float across = f[0] / c->wide[0], down = f[1] / c->wide[1];
    c->view_window[0] *= across;
    c->view_window[1] *= down;
    widen(GUEST(GliCamera, camera), across, down);
    c->wide[0] = f[0];
    c->wide[1] = f[1];
    revo_log("[gli] camera widened again (x%g across, x%g down)%s", (double)f[0], (double)f[1], c == &g_second ? " (the overlays')" : "");
}
