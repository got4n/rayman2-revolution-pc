#ifndef GLI_GLI_H
#define GLI_GLI_H

#include "gliglou.h"

s32 gli_open(GuestPtr init);
void gli_open_rasterizer(void);
void gli_create_default_heap(u32 size);
void gli_create_work_heap(u32 size);

void gli_clear(s32 depth);
void gli_clear_unless_paused(void);
void gli_background(GuestPtr color);
void gli_show_frame(void);
s32 gli_before_drawing(void);

typedef struct Camera {
    u32 handle;
    float view_window[2];
    float near_plane, far_plane;
    float view[4][4];
    GuestPtr gli;
    float wide[2];
} Camera;

void cameras_create(void);
Camera *camera_current(void);
Camera *camera_second(void);
float cameras_main_near(void);
float cameras_near_screen_z(void);
float camera_screen_z(const Camera *camera, float q);
void camera_depth_terms(const Camera *camera, float *scale, float *offset);
GuestPtr gld_camera_matrix(GuestPtr viewport);
void gli_viewport_camera(GuestPtr viewport);
void gli_camera_fov(GuestPtr camera);

void gli_select_material(void);

void state_blend(enum Blend blend);
void state_mode(u8 mode);
void state_select_texture(void);
void state_quads(void);
void state_forget_addressing(void);
void state_z_write(bool on);
void state_z_test(bool on);
void state_forget_blend(void);
void state_fog(bool on);
void state_forget_fog(void);
void state_apply_fog(GuestPtr fog);

s32 gli_load_level_textures(GuestPtr level);

typedef struct Texture {
    const struct TxdTexture *file;
    u32 handle;
    u32 raster;
    u32 image;
    u32 lightmap_image;
    u8 filter;
    u8 address_u, address_v;
} Texture;

enum { ADDRESS_WRAP = 1, ADDRESS_CLAMP = 3 };

Texture *textures_find(const char *name);
Texture *textures_take(const char *name);
void textures_purge(void);
int textures_count(void);
void textures_texels(const struct TxdTexture *t, u32 *out);

s32 gli_load_level_clump(GuestPtr level);

struct Clump;
const struct Clump *level_clump(void);

void gli_create_level_objects(void);
void gli_destroy_level_objects(void);

typedef struct Frame {
    u32 handle;
    float right[3], up[3], at[3], pos[3];
    struct Frame *parent;
} Frame;

void frame_from_matrix(Frame *f, const float m[4][4]);

struct ClumpGeometry;
typedef struct Atomic {
    u32 handle;
    const struct ClumpGeometry *geometry;
    u8 flags;
    u32 material;
    Frame *frame;
    int lightmap;
} Atomic;

enum { RW_LIGHT_DIRECTIONAL = 1, RW_LIGHT_AMBIENT = 2, RW_LIGHT_POINT = 0x80 };

typedef struct Light {
    u32 handle;
    bool on;
    u8 type;
    float color[4];
    float radius, cone_angle;
    Frame *frame;
} Light;

Light *const *level_lights(int *count);
Light *level_ambient(void);
struct ClumpGeometry *level_geometry(const Atomic *a);

#define LIGHT_LIST_SIZE 16
typedef struct LightList {
    GuestPtr lights[LIGHT_LIST_SIZE];
    int count;
} LightList;

void gli_ambient_color(GuestPtr color);
void gli_ambient_off(void);
void gli_light_mask(s32 mask);
u32 gli_remove_light(u32 driver_light);
void gli_light_matrix(GuestPtr light);
void gli_light_color(GuestPtr light, GuestPtr color);
void gli_light_cone_angle(GuestPtr light);
void gli_light_radius(GuestPtr light);
void lights_all_out(void);
void lights_of_viewport_in(void);
void lights_of_viewport(LightList *list);
void lights_put_in(const LightList *list);
void lights_recount(void);
struct ClumpGeometry;
s32 lights_mask(void);
s32 lights_viewport_count(void);
GuestPtr lights_viewport(int i);

void gli_send_object(void);
void gli_object_focal(float focal);
void gli_lock_lightmap_rasters(void);
void gli_draw_queued(GuestPtr viewport);
typedef struct QuadVertex {
    float pos[3];
    u8 color[4];
    float u, v;
} QuadVertex;
typedef struct Quad { QuadVertex v[4]; } Quad;

void draw_quads(const Quad *quads, int count);
void draw_world_triangles(const QuadVertex *v, int count);

typedef struct ShadowView {
    float right[3], up[3], at[3], pos[3];
    float scale[2];
    float near_plane;
    int width, height;
    u32 target;
    u8 color[4];
    bool textured;
} ShadowView;

void draw_into_shadow(const ShadowView *view);

void draw_atomic(const Atomic *a, const Frame *frame, bool lightmapped);
struct GsView;
const struct GsView *draw_camera_view(struct GsView *out);

typedef struct Material {
    u32 handle;
    u8 color[4];
    u32 texture;
} Material;

u32 gli_create_material(void);
void gli_material_color(u32 material, GuestPtr color);
u32 gli_material_texture(u32 material, u32 texture);
void gli_destroy_material(u32 material);

void screen_init(void);
void screen_im2d_triangles(GuestPtr vertices, GuestPtr indices, int count);
void gli_2d_letter(GuestPtr size, GuestPtr material, float x0, float y0, float x1, float y1);
void gli_2d_prepare_rectangle(GuestPtr p0, GuestPtr p1, GuestPtr size, float alpha, float grey);
void gli_2d_render_prepared(void);
void screen_quad(float x0, float y0, float x1, float y1, const float rgba[4]);

u32 icons_image(GuestPtr material);
void icons_forget(void);

#define ATOMIC_VIEW_GEOMETRY 0x18
struct ClumpGeometry;
u32 geometry_handle(struct ClumpGeometry *g);
void morphs_clear(void);
Atomic *const *level_atomics(int *count);
u32 morph_get_target(u32 geometry, s32 index);
u32 morph_get_vertices(u32 target);
u32 morph_get_normals(u32 target);
u32 morph_lock(u32 geometry, u32 flags);
u32 morph_vertex_count(u32 geometry);
u32 morph_unlock(u32 geometry);
void gli_object_wobble(GuestPtr object);

typedef struct Raster {
    u32 handle;
    int width, height;
    u32 *texels;
    u32 texture;
} Raster;
u32 raster_texture(Raster *r);
Raster *raster_new(int width, int height);
void raster_free(Raster *r);
void gli_raster_read_image(u32 raster, GuestPtr name);
void gli_startup_image(GuestPtr name, s32 width, s32 height, u8 flip);
void gli_show_raster(void);

void gli_fade_copy(void);
void gli_video_frame(void);

s32 gli_shadows_open(u32 count128, u32 count64);
void gli_shadows_close(void);
void gli_shadows_begin(void);
void gli_shadows_end(void);
void gli_shadow_character(GuestPtr object, GuestPtr matrix, GuestPtr material);
s32 gli_shadow_buffer(GuestPtr out, s32 count);
void gli_shadow_owner(GuestPtr sound, GuestPtr character);
void gli_shadow_mesh(GuestPtr mesh, GuestPtr position);
void gli_shadow_mesh_end(void);
void gli_shadow_vertex(GuestPtr position, GuestPtr uv, float alpha);
void gli_shadow_object(GuestPtr object, u32 render_flags);
void gli_shadow_draw(GuestPtr mesh, GuestPtr fog);

void halos_record(GuestPtr object, GuestPtr element);
void gli_draw_halos(GuestPtr viewport);

void gli_2d_reset(void);
void gli_2d_add_rectangle(s32 x0, s32 y0, s32 x1, s32 y1);
s32 gli_2d_render(void);
void twod_flush(void);

void world_create(void);
const float *world_origin(void);
bool world_exists(void);
void queue_clear_objects(void);
void queue_init(const Quad *quad);
void sprites_init(void);
void sprites_draw_quad(GuestPtr viewport, const Quad *quad, GuestPtr texture);
void gli_draw_sprites(GuestPtr viewport);
void gli_sprite_elements(GuestPtr viewport, GuestPtr list);
u8 gli_part_facing(GuestPtr center, GuestPtr material, GuestPtr viewport, u32 count, float hw, float hh);
u8 gli_part_flat(GuestPtr center, GuestPtr material, GuestPtr viewport, u32 count, float hw, float hh);
u8 gli_part_oriented(float s0, float s1, GuestPtr center, GuestPtr material, GuestPtr viewport, u32 count,
                     GuestPtr axis0, GuestPtr axis1);
void gli_part_quad(GuestPtr c0, GuestPtr c1, GuestPtr c2, GuestPtr c3, GuestPtr material);
Quad *queue_quad(GuestPtr texture);

#endif
