#ifndef GLI_RENDER_H
#define GLI_RENDER_H

#include "guest.h"

typedef enum { FACTOR_ZERO, FACTOR_ONE, FACTOR_SRC_ALPHA, FACTOR_INV_SRC_ALPHA } BlendFactor;

typedef struct RenderState {
    u32 texture;
    bool wrap_u, wrap_v;
    bool linear_filter;
    bool vertex_alpha;
    BlendFactor src, dst;
    bool z_test, z_write;
    bool fog;
    float fog_start, fog_end;
    u32 fog_color;
} RenderState;

extern RenderState render_state;

void render_blend(bool vertex_alpha, BlendFactor src, BlendFactor dst);
void render_z_test(bool on);
void render_z_write(bool on);
void render_fog(bool on);
bool render_fog_on(void);
void render_fog_range(float start, float end);
void render_fog_color(u32 argb);
u32 render_fog_color_argb(void);
void render_texture_set(u32 texture);
void render_addressing(bool wrap_u, bool wrap_v);
void render_linear_filter(bool on);

#define GS_WIDTH 640
#define GS_HEIGHT 448

typedef enum { GS_TRIANGLES, GS_TRISTRIP, GS_TRIFAN, GS_SPRITE, GS_COPY } GsPrim;

enum { GS_CS = 0, GS_CD = 1, GS_ZERO = 2 };
enum { GS_AS = 0, GS_AD = 1, GS_FIX = 2 };
enum { GS_NEVER, GS_ALWAYS, GS_LESS, GS_LEQUAL, GS_EQUAL, GS_GEQUAL, GS_GREATER, GS_NOTEQUAL };
enum { GS_KEEP, GS_FB_ONLY, GS_ZB_ONLY, GS_RGB_ONLY };
enum { GS_Z_NEVER, GS_Z_ALWAYS, GS_Z_GEQUAL, GS_Z_GREATER };

enum { GS_MODULATE, GS_DECAL, GS_HIGHLIGHT, GS_HIGHLIGHT2 };
enum { GS_REPEAT, GS_CLAMP, GS_REGION_CLAMP, GS_REGION_REPEAT };

typedef struct GsState {
    bool flat;
    bool scissor;
    u16 scissor_box[4];
    bool tme;
    u32 texture;
    u8 tfx;
    bool tcc;
    u8 wrap_s, wrap_t;
    u16 region[4];
    u8 mmag, mmin;
    bool lcm;
    u8 l;
    s16 k;
    bool pabe;
    bool blend;
    u8 a, b, c, d, fix;
    bool alpha_test;
    u8 atst, aref, afail;
    u8 ztst;
    bool zwrite;
    bool fog;
    u32 fog_color;
    u32 fbmsk;
    bool colclamp;
    bool fba;
    u32 target;
    u32 lightmap;
} GsState;

#define GS_TARGET_SHOWN 0xFFFFFFFFu

typedef struct GsVertex {
    float x, y, z;
    float s, t, q;
    u8 r, g, b, a;
    u8 fog, pad[3];
    u32 key;
} GsVertex;

void render_draw(GsPrim prim, const GsState *state, const GsVertex *v, int n);

typedef struct GsVertexExt {
    float lu, lv;
} GsVertexExt;

void render_draw_ext(GsPrim prim, const GsState *state, const GsVertex *v, const GsVertexExt *ext, int n);
int render_texture_filter(void);
typedef struct GsView {
    float view[4][3];
    float window[2];
    float origin[3];
} GsView;

void render_set_source(u32 source, const GsView *view);
#define GS_SOURCE_NONE 0xFFFFFFFFu

u32 render_texture_new(int width, int height, const u32 *texels);

typedef struct GsTexture {
    int width, height;
    u32 *texels;
    bool target;
    void *vk;
    void *vk_target;
} GsTexture;
GsTexture *render_texture(u32 texture);
u32 render_target_new(int width, int height);
void render_texture_free(u32 texture);

float widescreen_factor(void);
void render_2d_widescreen(GsVertex *v, int n);
void render_2d_hud(GsVertex *v, int n);
int render_hud_at_sides(void);
void render_2d_widescreen_centre(GsVertex *v, int n);

GsState render_gs_state(void);
GsState render_gs_state_textured(void);
u8 render_gs_channel(float c);

void render_clear_depth(void);
void render_clear_color(u32 argb);
void render_background(u32 argb);
typedef struct GliRectangle { s32 x0, y0, x1, y1; } GliRectangle;
void render_rectangles(const GliRectangle *r, int n, float z);
void render_show_frame(void);
void render_copy_shown(void);
u64 render_frames_shown(void);

typedef struct GsProjection {
    float window[2];
    float depth_scale, depth_offset;
    float near_plane;
} GsProjection;

void render_set_projection(const GsProjection *p);

typedef struct GsDraw {
    GsPrim prim;
    GsState state;
    u32 first, count;
    u32 source;
    bool has_view;
    GsView view;
    bool camera;
    GsProjection projection;
} GsDraw;

typedef struct GsFrame {
    GsDraw *draws;
    u32 ndraws, draws_cap;
    GsVertex *vertices;
    u32 nvertices, vertices_cap;
    GsVertexExt *ext;
    u32 next, ext_cap;
    u64 number;
    int buffer;
    bool logged;
} GsFrame;

const GsFrame *render_next_frame(void);
bool render_wait_frame(int ms);

double now_ms(void);
void render_textures_release(u64 number, void (*release)(GsTexture *t));

#endif
