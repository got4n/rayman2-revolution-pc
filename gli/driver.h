#ifndef GLI_DRIVER_H
#define GLI_DRIVER_H

#include "gli.h"

#define PC_DRIVER(address)

PC_DRIVER(0x0011C9C0) s32 drv_open(u32 a0, u32 a1, GuestPtr init);
PC_DRIVER(0x0011C588) void gli_open_rasterizer(void);
PC_DRIVER(0x00157C68) void gli_create_default_heap(u32 size);
PC_DRIVER(0x00157C70) void gli_create_work_heap(u32 size);
PC_DRIVER(0x0011C5D0) void drv_close_rasterizer(void);

PC_DRIVER(0x0011C5D8) void gli_show_frame(void);
PC_DRIVER(0x0011C628) void gli_clear(s32 depth);
PC_DRIVER(0x0011C668) void gli_clear_unless_paused(void);
PC_DRIVER(0x0014FEE0) void gli_viewport_camera(GuestPtr viewport);
PC_DRIVER(0x00153800) void gli_background(GuestPtr color);
PC_DRIVER(0x0011BC98) s32 gli_before_drawing(void);
PC_DRIVER(0x0011CEF8) void gli_camera_fov(GuestPtr camera);

PC_DRIVER(0x0014DAB8) s32 gli_load_level_clump(GuestPtr level);
PC_DRIVER(0x0014F318) s32 gli_load_level_textures(GuestPtr level);
PC_DRIVER(0x0014D7D8) void gli_create_level_objects(void);
PC_DRIVER(0x0014DA48) void gli_destroy_level_objects(void);
PC_DRIVER(0x001516F8) void gli_lock_lightmap_rasters(void);

PC_DRIVER(0x0011BCC0) void gli_select_material(void);
PC_DRIVER(0x0014ED88) u32 gli_create_material(void);
PC_DRIVER(0x0014EE38) void gli_material_color(u32 material, GuestPtr color);
PC_DRIVER(0x0016FA18) u32 gli_material_texture(u32 material, u32 texture);

PC_DRIVER(0x0011DA98) void gli_object_focal(float focal);
PC_DRIVER(0x0014E068) void gli_object_wobble(GuestPtr object);
PC_DRIVER(0x0014D010) void gli_fade_copy(void);
PC_DRIVER(0x00215668) void gli_video_frame(void);
PC_DRIVER(0x0014E0D0) void gli_draw_halos(GuestPtr viewport);
PC_DRIVER(0x0014E9C0) void gli_ambient_color(GuestPtr color);
PC_DRIVER(0x0014EA00) void gli_ambient_off(void);
PC_DRIVER(0x0014EA38) void gli_light_mask(s32 mask);
PC_DRIVER(0x0014EBE0) u32 gli_remove_light(u32 driver_light);
PC_DRIVER(0x0014EC78) void lights_all_out(void);
PC_DRIVER(0x00152978) void gli_draw_queued(GuestPtr viewport);
PC_DRIVER(0x00152B10) void gli_send_object(void);
PC_DRIVER(0x0014E7A0) void gli_light_matrix(GuestPtr light);
PC_DRIVER(0x0014E810) void gli_light_color(GuestPtr light, GuestPtr color);
PC_DRIVER(0x0014E830) void gli_light_cone_angle(GuestPtr light);
PC_DRIVER(0x0014E850) void gli_light_radius(GuestPtr light);

PC_DRIVER(0x0011B6A8) void gli_2d_reset(void);
PC_DRIVER(0x0011B6B0) void gli_2d_add_rectangle(s32 x0, s32 y0, s32 x1, s32 y1);
PC_DRIVER(0x0011BC18) s32 gli_2d_render(void);
PC_DRIVER(0x001556E8) void gli_2d_prepare_rectangle(GuestPtr p0, GuestPtr p1, GuestPtr size, float alpha,
                                                    float grey);
PC_DRIVER(0x00155808) void gli_2d_render_prepared(void);
PC_DRIVER(0x001552A8) void gli_2d_letter(GuestPtr size, GuestPtr material, float x0, float y0, float x1, float y1);
PC_DRIVER(0x00153340) void gli_raster_read_image(u32 raster, GuestPtr name);
PC_DRIVER(0x00153428) void drv_startup_image(GuestPtr name, s32 width, s32 height, u32 flip);
PC_DRIVER(0x00153550) void gli_show_raster(void);
PC_DRIVER(0x00155D68) void gli_draw_sprites(GuestPtr viewport);
PC_DRIVER(0x00155FD8) void gli_sprite_elements(GuestPtr viewport, GuestPtr list);

PC_DRIVER(0x00156800) void gli_part_quad(GuestPtr c0, GuestPtr c1, GuestPtr c2, GuestPtr c3, GuestPtr material);
PC_DRIVER(0x00156AD0) u8 gli_part_facing(GuestPtr center, GuestPtr material, GuestPtr viewport, u32 count, float hw,
                                         float hh);
PC_DRIVER(0x00156E98) u8 gli_part_flat(GuestPtr center, GuestPtr material, GuestPtr viewport, u32 count, float hw,
                                       float hh);
PC_DRIVER(0x00157190) u8 gli_part_oriented(float s0, float s1, GuestPtr center, GuestPtr material, GuestPtr viewport,
                                           u32 count, GuestPtr axis0, GuestPtr axis1);

PC_DRIVER(0x00153D80) s32 gli_shadows_open(u32 count128, u32 count64);
PC_DRIVER(0x00153F48) void gli_shadows_close(void);
PC_DRIVER(0x00154750) void gli_shadows_begin(void);
PC_DRIVER(0x00154450) void gli_shadow_character(GuestPtr object, GuestPtr matrix, GuestPtr material);
PC_DRIVER(0x001547F8) void gli_shadows_end(void);
PC_DRIVER(0x00151D28) s32 gli_shadow_buffer(GuestPtr out, s32 count);
PC_DRIVER(0x00151D88) void gli_shadow_owner(GuestPtr sound, GuestPtr character);
PC_DRIVER(0x00151D90) void gli_shadow_mesh(GuestPtr mesh, GuestPtr position);
PC_DRIVER(0x00151DD0) void gli_shadow_mesh_end(void);
PC_DRIVER(0x00151DE0) void gli_shadow_vertex(GuestPtr position, GuestPtr uv, float alpha);
PC_DRIVER(0x00151EC0) void gli_shadow_object(GuestPtr object, u32 render_flags);
PC_DRIVER(0x00151F58) void gli_shadow_draw(GuestPtr mesh, GuestPtr fog);

PC_DRIVER(0x0018C590) u32 rw_camera_begin_update(GuestPtr camera);
PC_DRIVER(0x0018C5B0) u32 rw_camera_end_update(GuestPtr camera);
PC_DRIVER(0x0018C618) u32 rw_camera_show_raster(GuestPtr camera, GuestPtr device, u32 flags);
PC_DRIVER(0x0018C5E0) u32 rw_camera_clear(GuestPtr camera, GuestPtr color, s32 flags);
PC_DRIVER(0x00193178) u32 rw_render_state_set(u32 state, u32 value);
PC_DRIVER(0x00193130) float rw_im2d_get_near_screen_z(void);
PC_DRIVER(0x0018CA38) float rw_camera_get_near_clip_plane(GuestPtr camera);
PC_DRIVER(0x00193248) u32 rw_im2d_render_indexed(s32 type, GuestPtr vertices, s32 count, GuestPtr indices,
                                                 s32 index_count);
PC_DRIVER(0x0016E2A0) u32 morph_get_target(u32 geometry, s32 index);
PC_DRIVER(0x0016E3D8) u32 morph_get_vertices(u32 target);
PC_DRIVER(0x0016E3E0) u32 morph_get_normals(u32 target);
PC_DRIVER(0x0016E470) u32 morph_lock(u32 geometry, u32 flags);
PC_DRIVER(0x0016E188) u32 morph_vertex_count(u32 geometry);
PC_DRIVER(0x0016CEA0) u32 morph_unlock(u32 geometry);
PC_DRIVER(0x001BA570) void rw_texture_cache_flush(void);
PC_DRIVER(0x00195D28) u32 rw_raster_create(s32 width, s32 height, s32 depth, u32 flags);
PC_DRIVER(0x001A98E0) void rw_open_local_packet(u32 type, u32 size);
PC_DRIVER(0x00195E20) u32 rw_raster_destroy(u32 raster);

#endif
