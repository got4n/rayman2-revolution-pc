#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "mods.h"
#include "render.h"

RenderState render_state = {
    .wrap_u = true, .wrap_v = true,
    .src = FACTOR_SRC_ALPHA, .dst = FACTOR_INV_SRC_ALPHA,
    .z_test = true, .z_write = true,
};

void render_blend(bool vertex_alpha, BlendFactor src, BlendFactor dst) {
    render_state.vertex_alpha = vertex_alpha;
    render_state.src = src;
    render_state.dst = dst;
}
void render_z_test(bool on) { render_state.z_test = on; }
void render_z_write(bool on) { render_state.z_write = on; }
void render_fog(bool on) { render_state.fog = on; }
bool render_fog_on(void) { return render_state.fog; }
void render_fog_range(float start, float end) { render_state.fog_start = start; render_state.fog_end = end; }
void render_fog_color(u32 argb) { render_state.fog_color = argb; }
u32 render_fog_color_argb(void) { return render_state.fog_color; }
void render_texture_set(u32 texture) { render_state.texture = texture; }

static GsTexture *g_textures[1 << 16];
static u32 g_ntextures;
static u32 g_reusable[1 << 16], g_nreusable;
typedef struct Freed { u32 texture; u64 shown; } Freed;
static Freed *g_freed;
static size_t g_nfreed, g_freed_cap;
static pthread_mutex_t g_texture_lock = PTHREAD_MUTEX_INITIALIZER;

u32 render_texture_new(int width, int height, const u32 *texels) {
    pthread_mutex_lock(&g_texture_lock);
    u32 id = g_nreusable ? g_reusable[--g_nreusable] : g_ntextures + 1 < (1u << 16) ? ++g_ntextures : 0;
    pthread_mutex_unlock(&g_texture_lock);
    if (!id) return 0;
    GsTexture *t = calloc(1, sizeof *t);
    t->width = width;
    t->height = height;
    t->texels = malloc(sizeof *texels * (size_t)width * (size_t)height);
    memcpy(t->texels, texels, sizeof *texels * (size_t)width * (size_t)height);
    g_textures[id] = t;
    return id;
}

u32 render_target_new(int width, int height) {
    pthread_mutex_lock(&g_texture_lock);
    u32 id = g_nreusable ? g_reusable[--g_nreusable] : g_ntextures + 1 < (1u << 16) ? ++g_ntextures : 0;
    pthread_mutex_unlock(&g_texture_lock);
    if (!id) return 0;
    GsTexture *t = calloc(1, sizeof *t);
    t->width = width;
    t->height = height;
    t->target = true;
    g_textures[id] = t;
    return id;
}

GsTexture *render_texture(u32 texture) { return texture && texture <= g_ntextures ? g_textures[texture] : NULL; }

static u64 g_shown;

void render_texture_free(u32 texture) {
    if (!render_texture(texture)) return;
    pthread_mutex_lock(&g_texture_lock);
    if (g_nfreed == g_freed_cap) {
        g_freed_cap = g_freed_cap ? g_freed_cap * 2 : 1024;
        g_freed = realloc(g_freed, sizeof *g_freed * g_freed_cap);
    }
    g_freed[g_nfreed++] = (Freed){texture, g_shown};
    pthread_mutex_unlock(&g_texture_lock);
}

void render_textures_release(u64 number, void (*release)(GsTexture *t)) {
    pthread_mutex_lock(&g_texture_lock);
    size_t kept = 0;
    for (size_t i = 0; i < g_nfreed; i++) {
        if (number <= g_freed[i].shown + 1) {
            g_freed[kept++] = g_freed[i];
            continue;
        }
        GsTexture *t = g_textures[g_freed[i].texture];
        if (release) release(t);
        free(t->texels);
        free(t);
        g_textures[g_freed[i].texture] = NULL;
        g_reusable[g_nreusable++] = g_freed[i].texture;
    }
    g_nfreed = kept;
    pthread_mutex_unlock(&g_texture_lock);
}
void render_addressing(bool wrap_u, bool wrap_v) { render_state.wrap_u = wrap_u; render_state.wrap_v = wrap_v; }
void render_linear_filter(bool on) { render_state.linear_filter = on; }

static void gs_blend(GsState *s, BlendFactor src, BlendFactor dst) {
    s->blend = true;
    s->c = GS_AS;
    if (src == FACTOR_SRC_ALPHA && dst == FACTOR_INV_SRC_ALPHA) {
        s->a = GS_CS; s->b = GS_CD; s->d = GS_CD;
    } else if (src == FACTOR_SRC_ALPHA && dst == FACTOR_ONE) {
        s->a = GS_CS; s->b = GS_ZERO; s->d = GS_CD;
    } else {
        s->a = GS_ZERO; s->b = GS_ZERO; s->d = GS_CS;
    }
}

GsState render_gs_state(void) {
    GsState s = {0};
    if (render_state.vertex_alpha) gs_blend(&s, render_state.src, render_state.dst);
    s.ztst = render_state.z_test ? GS_Z_GEQUAL : GS_Z_ALWAYS;
    s.zwrite = render_state.z_write;
    s.fog = render_fog_on();
    u32 c = render_state.fog_color;
    s.fog_color = (c >> 16 & 0xFF) | (c & 0xFF00) | (c & 0xFF) << 16;
    s.colclamp = true;
    return s;
}

static u8 gs_channel(u32 c) { return (u8)((c * 128 + 127) / 255); }

u8 render_gs_channel(float c) {
    float scaled = fmul(c <= 0.0f ? 0.0f : c >= 255.0f ? 255.0f : c, 128.0f / 255.0f);
    return (u8)fcvtw(scaled);
}

GsState render_gs_state_textured(void) {
    GsState s = render_gs_state();
    const GsTexture *t = render_texture(render_state.texture);
    if (!t) return s;
    s.tme = true;
    s.texture = render_state.texture;
    s.tfx = GS_MODULATE;
    s.tcc = true;
    s.wrap_s = render_state.wrap_u ? GS_REPEAT : GS_CLAMP;
    s.wrap_t = render_state.wrap_v ? GS_REPEAT : GS_CLAMP;
    s.mmag = s.mmin = render_state.linear_filter;
    s.k = -4 * 16;
    s.alpha_test = true;
    s.atst = GS_GEQUAL;
    s.aref = 64;
    s.afail = GS_FB_ONLY;
    return s;
}

static GsVertex gs_vertex(float x, float y, float z, u32 argb) {
    GsVertex v = {.x = x, .y = y, .z = z, .q = 1.0f, .fog = 255};
    v.r = gs_channel(argb >> 16 & 0xFF);
    v.g = gs_channel(argb >> 8 & 0xFF);
    v.b = gs_channel(argb & 0xFF);
    v.a = gs_channel(argb >> 24);
    return v;
}

#define FRAMES 8
#define AHEAD 1
#define WAIT_MS 250

static GsFrame g_frames[FRAMES];
static int g_building;
static int g_waiting[FRAMES], g_first, g_nwaiting;
static int g_drawing = -1;
static bool g_drawn;
static volatile int g_fast;

void render_fast_forward(int on) { g_fast = on; }
static int g_draw_buffer;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_room = PTHREAD_COND_INITIALIZER;
static pthread_cond_t g_ready = PTHREAD_COND_INITIALIZER;

static u32 g_source;
static bool g_has_view;
static GsView g_view;
static bool g_camera;
static GsProjection g_projection;

void render_set_projection(const GsProjection *p) {
    g_camera = p != NULL;
    if (p) g_projection = *p;
}

void render_draw(GsPrim prim, const GsState *state, const GsVertex *v, int n) {
    GsFrame *f = &g_frames[g_building];
    if (f->nvertices + (u32)n > f->vertices_cap) {
        f->vertices_cap = (f->nvertices + (u32)n) * 2 + 1024;
        f->vertices = realloc(f->vertices, sizeof *f->vertices * f->vertices_cap);
    }
    if (f->ndraws == f->draws_cap) {
        f->draws_cap = f->draws_cap * 2 + 64;
        f->draws = realloc(f->draws, sizeof *f->draws * f->draws_cap);
    }
    if (n) memcpy(f->vertices + f->nvertices, v, sizeof *v * (size_t)n);
    f->draws[f->ndraws++] = (GsDraw){prim, *state, f->nvertices, (u32)n, g_source, g_has_view, g_view, g_camera,
                                     g_projection};
    f->nvertices += (u32)n;
    g_source = 0;
    g_has_view = false;
    g_camera = false;
}

void render_draw_ext(GsPrim prim, const GsState *state, const GsVertex *v, const GsVertexExt *ext, int n) {
    GsFrame *f = &g_frames[g_building];
    const u32 at = f->nvertices, need = at + (u32)n;
    if (need > f->ext_cap) {
        f->ext_cap = need * 2 + 1024;
        f->ext = realloc(f->ext, sizeof *f->ext * f->ext_cap);
    }
    if (f->next < at) memset(f->ext + f->next, 0, sizeof *f->ext * (at - f->next));
    if (n) memcpy(f->ext + at, ext, sizeof *ext * (size_t)n);
    f->next = need;
    render_draw(prim, state, v, n);
}

int render_texture_filter(void) {
    static const int taps[5] = {0, 1, 4, 8, 16};
    const int v = revo_set(REVO_SET_TEXTURE_FILTER);
    return v > 0 && v < 5 ? taps[v] : 0;
}

void render_set_source(u32 source, const GsView *view) {
    g_source = source;
    g_has_view = view != NULL;
    if (view) g_view = *view;
}

u64 revo_frame_count(void);

static bool log_frame(const GsFrame *f, u64 number) { return false; }

static bool frame_free(int k) {
    if (k == g_building || k == g_drawing) return false;
    for (int i = 0; i < g_nwaiting; i++)
        if (g_waiting[(g_first + i) % FRAMES] == k) return false;
    return true;
}

static int free_frame(void) {
    for (int k = 0; k < FRAMES; k++)
        if (frame_free(k)) return k;
    return -1;
}

void render_show_frame(void) {
    GsFrame *done = &g_frames[g_building];
    done->logged = log_frame(done, g_shown + 1);
    pthread_mutex_lock(&g_lock);
    done->number = ++g_shown;
    g_waiting[(g_first + g_nwaiting++) % FRAMES] = g_building;
    pthread_cond_signal(&g_ready);
    g_building = -1;
    int k = free_frame();
    if ((k < 0 || g_nwaiting > AHEAD) && g_drawn && !g_fast) {
        struct timespec until;
        clock_gettime(CLOCK_REALTIME, &until);
        until.tv_nsec += WAIT_MS * 1000000L;
        until.tv_sec += until.tv_nsec / 1000000000L;
        until.tv_nsec %= 1000000000L;
        while ((k = free_frame()) < 0 || g_nwaiting > AHEAD)
            if (pthread_cond_timedwait(&g_room, &g_lock, &until)) break;
    }
    if (k < 0) {
        k = g_waiting[g_first];
        g_first = (g_first + 1) % FRAMES;
        g_nwaiting--;
    }
    g_building = k;
    pthread_mutex_unlock(&g_lock);
    g_draw_buffer ^= 1;
    g_frames[k].ndraws = 0;
    g_frames[k].nvertices = 0;
    g_frames[k].next = 0;
    g_frames[k].buffer = g_draw_buffer;
}

void render_copy_shown(void) {
    GsState s = {0};
    render_draw(GS_COPY, &s, NULL, 0);
}

u64 render_frames_shown(void) { return g_shown; }

bool render_wait_frame(int ms) {
    pthread_mutex_lock(&g_lock);
    if (!g_nwaiting) {
        struct timespec until;
        clock_gettime(CLOCK_REALTIME, &until);
        until.tv_nsec += ms * 1000000L;
        until.tv_sec += until.tv_nsec / 1000000000L;
        until.tv_nsec %= 1000000000L;
        while (!g_nwaiting)
            if (pthread_cond_timedwait(&g_ready, &g_lock, &until)) break;
    }
    const bool ready = g_nwaiting > 0;
    pthread_mutex_unlock(&g_lock);
    return ready;
}

const GsFrame *render_next_frame(void) {
    pthread_mutex_lock(&g_lock);
    g_drawn = true;
    g_drawing = -1;
    if (g_nwaiting) {
        g_drawing = g_waiting[g_first];
        g_first = (g_first + 1) % FRAMES;
        g_nwaiting--;
    }
    int d = g_drawing;
    pthread_cond_broadcast(&g_room);
    pthread_mutex_unlock(&g_lock);
    return d >= 0 ? &g_frames[d] : NULL;
}

void render_clear_depth(void) {
    GsState s = {.ztst = GS_Z_ALWAYS, .zwrite = true, .fbmsk = 0xFFFFFFFFu, .colclamp = true};
    GsVertex v[2] = {gs_vertex(0, 0, 0, 0), gs_vertex(GS_WIDTH, GS_HEIGHT, 0, 0)};
    render_draw(GS_SPRITE, &s, v, 2);
}

void render_clear_color(u32 argb) {
    GsState s = {.ztst = GS_Z_ALWAYS, .colclamp = true};
    GsVertex v[2] = {gs_vertex(0, 0, 0, argb), gs_vertex(GS_WIDTH, GS_HEIGHT, 0, argb)};
    render_draw(GS_SPRITE, &s, v, 2);
}

void render_background(u32 argb) {
    GsState s = render_gs_state();
    GsVertex v[4] = {gs_vertex(0, 0, 0, argb), gs_vertex(0, GS_HEIGHT, 0, argb),
                     gs_vertex(GS_WIDTH, 0, 0, argb), gs_vertex(GS_WIDTH, GS_HEIGHT, 0, argb)};
    render_draw(GS_TRISTRIP, &s, v, 4);
}

float widescreen_factor(void) {
    int across, down;
    revo_view_aspect(&across, &down);
    return ((float)across / (float)down) / (4.0f / 3.0f);
}

static void widescreen_2d(GsVertex *v, int n) {
    const float wide = widescreen_factor();
    if (wide == 1.0f || n <= 0) return;
    float x0 = v[0].x, x1 = v[0].x;
    for (int i = 1; i < n; i++) {
        x0 = v[i].x < x0 ? v[i].x : x0;
        x1 = v[i].x > x1 ? v[i].x : x1;
    }
    if (x0 <= 1.0f && x1 >= GS_WIDTH - 1.0f) return;
    for (int i = 0; i < n; i++) v[i].x = GS_WIDTH * 0.5f + (v[i].x - GS_WIDTH * 0.5f) / wide;
}

int render_hud_at_sides(void) { return widescreen_factor() != 1.0f && revo_set(REVO_SET_HUD_EDGES); }

void render_2d_widescreen(GsVertex *v, int n) { widescreen_2d(v, n); }
void render_2d_widescreen_centre(GsVertex *v, int n) { widescreen_2d(v, n); }
void render_2d_hud(GsVertex *v, int n) {
    if (!render_hud_at_sides()) widescreen_2d(v, n);
}

void render_rectangles(const GliRectangle *r, int n, float z) {
    GsState s = render_gs_state();
    for (int i = 0; i < n; i++) {
        GsVertex v[4] = {gs_vertex((float)r[i].x0, (float)r[i].y1, z, 0), gs_vertex((float)r[i].x0, (float)r[i].y0, z, 0),
                         gs_vertex((float)r[i].x1, (float)r[i].y1, z, 0), gs_vertex((float)r[i].x1, (float)r[i].y0, z, 0)};
        render_2d_widescreen(v, 4);
        render_draw(GS_TRISTRIP, &s, v, 4);
    }
}
