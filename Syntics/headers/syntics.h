#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <immintrin.h>

#include "vulkan/sy_vulkan.h"

#ifdef LINUX

#include <xcb/xcb.h>
#include <xcb/xfixes.h>
#include <xcb/xcb_cursor.h>
#include <vulkan/vulkan_xcb.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define thread_return_value void*
#define File_Change_Handle void*
#define Thread_Handle pthread_t
#define Mutex pthread_mutex_t
#define Semaphore sem_t

#define MAX_PATH 260

#define sysprintf(...) snprintf(__VA_ARGS__)
#define syscanf(...) sscanf(__VA_ARGS__)
#define sy_gcvt(buffer, buffer_size, val, num_digits)                          \
    gcvt(val, num_digits, buffer);

#else
#if 0
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else

#include "win32/sy_windows.h"
#include <vulkan/vulkan_win32.h>

#define thread_return_value unsigned long
#define File_Change_Handle HANDLE
#define Thread_Handle HANDLE
#define Mutex HANDLE
#define Semaphore HANDLE

#define sysprintf(...) sprintf_s(__VA_ARGS__)
#define syscanf(...) sscanf_s(__VA_ARGS__)
#define sy_gcvt(...) _gcvt_s(__VA_ARGS__);

#endif
#endif

#include <stb/stb_image_min.h>
#include <stb/stb_truetype.h>

#include "defines.h"
#include "math/syntics_math.h"
#include "vulkan_types.h"

#include "ansi_keycodes.h"
#include "event_system.h"
#include "region_alloc.h"
#include "lookup_table.h"
#include "entity.h"
#include "font.h"
#include "render_util.h"
#include "simple_particle.h"
#include "camera.h"
#include "logging.h"
#include "collision.h"
#include "gui.h"
#include "game.h"
#include "obj_load.h"
#include "notebook.h"

typedef struct File_Attrib
{
    u8* buffer;
    u32 current_pos;
    u32 size;
} File_Attrib;

typedef enum Visible_Local
{
    VERTEX_INDEX_VISIBLE_VISIBLE,
    VERTEX_INDEX_VISIBLE_LOCAL,
    VERTEX_INDEX_LOCAL_VISIBLE,
    VERTEX_INDEX_LOCAL_LOCAL,
} Visible_Local;

#ifdef DEBUG
global const b8 VALIDATIONS_ENABLE = true;
#else
global const b8 VALIDATIONS_ENABLE = false;
#endif

global char* WORKING_DIR = NULL;
global u32 WORKING_DIR_LEN = 0;

#include "syntics_app.h"
#include "frame_data.h"

// START_GENERATING| //
///////// | .\Syntics\src\application.c | //////////////////////

void application_init(u32 stack_size, u64 main_region_size, u16 app_width,
                      u16 app_height, u32 thread_pool_queue_size,
                      b8 full_screen, u32 event_count,
                      u32 vulkan_frames_in_flight, Render_State** render_state,
                      Application_State** app);

Application_Frame application_frame_create();

Application_Frame application_begin_frame(Application_Frame app_frame);

///////// | .\Syntics\src\buffers.c | //////////////////////

i32 type_index_get(VkPhysicalDeviceMemoryProperties mem_props,
                   VkMemoryRequirements mem_req,
                   VkMemoryPropertyFlags wanted_mem_props);

void mem_map_copy(VkDevice device, Buffer* buffer, void* data);

void mem_map_copy_index(VkDevice device, Index_Buffer* ib);

void mem_map_copy_vertex(VkDevice device, Vertex_Buffer* vb);

void mem_map_copy_unmap(VkDevice device, Buffer* buffer, void* data);

void memory_allocate(VkDevice device, VkPhysicalDevice physical_device,
                     VkMemoryRequirements mem_req,
                     VkMemoryPropertyFlags wanted_mem_props,
                     VkDeviceMemory* memory);

void commandbuffers_allocate(VkDevice device, VkCommandPool command_pool,
                             VkCommandBufferLevel level,
                             u32 command_buffer_count,
                             VkCommandBuffer* command_buffer);

void buffer_destroy(VkDevice device, Buffer buffer);

void texture_destroy(VkDevice device, Texture texture);

void image_destroy(VkDevice device, Image image);

void buffers_update(VkDevice device, Buffer* buffer, void* data,
                    size_t size_bytes);

VkCommandBuffer command_buffer_begin(VkDevice device,
                                     VkCommandPool command_pool,
                                     VkCommandBufferLevel level);

void command_buffer_end(VkDevice device, VkCommandPool command_pool,
                        VkCommandBuffer command_buff, VkQueue graphics_queue);

void buffer_copy(VkDevice device, VkCommandPool command_pool,
                 VkBuffer src_buffer, VkBuffer dst_buffer,
                 VkQueue graphics_queue, VkDeviceSize size_bytes);

void create_alloc_bind(VkDevice device, VkPhysicalDevice physical_device,
                       VkMemoryPropertyFlags wanted_mem_props,
                       VkBufferUsageFlags usage_flags, VkBuffer* buffer,
                       VkDeviceMemory* buffer_memory, VkDeviceSize data_size);

void staging_buffer_create(VkDevice device, VkPhysicalDevice physical_device,
                           void* data, VkDeviceSize size_bytes,
                           VkBufferUsageFlags usage_flags, Buffer* buffer);

void staging_buffer_to_local(VkDevice device, VkPhysicalDevice physical_device,
                             VkCommandPool command_pool, VkQueue graphics_queue,
                             VkBufferUsageFlags vertex_or_index, void* data,
                             VkBuffer* buffer, VkDeviceMemory* buffer_memory,
                             VkDeviceSize size_bytes);

void vertex_buffer_create_test(VkDevice device,
                               VkPhysicalDevice physical_device,
                               Vertex_Buffer* vertex_buffer);

void vertex_buffer_create_visible(VkDevice device,
                                  VkPhysicalDevice physical_device,
                                  Vertex_Buffer* vertex_buffer);

void vertex_buffer_create_local(VkDevice device,
                                VkPhysicalDevice physical_device,
                                VkCommandPool command_pool,
                                VkQueue graphics_queue,
                                Vertex_Buffer* vertex_buffer);

void create_index_buffer_test(VkDevice device, VkPhysicalDevice physical_device,
                              Index_Buffer* index_buffer);

void index_buffer_create_visible(VkDevice device,
                                 VkPhysicalDevice physical_device,
                                 Index_Buffer* index_buffer);

void index_buffer_create_local(VkDevice device,
                               VkPhysicalDevice physical_device,
                               VkCommandPool command_pool,
                               VkQueue graphics_queue,
                               Index_Buffer* index_buffer);

void vertex_index_buffer_create_default(VkDevice device,
                                        VkPhysicalDevice physical_device,
                                        VkCommandPool command_pool,
                                        VkQueue graphics_queue,
                                        Visible_Local visible_local,
                                        Vertex_Buffer* vertex_buffer,
                                        Index_Buffer* index_buffer);

void vertex_index_buffer_create_default1(
    VkDevice device, VkPhysicalDevice physical_device,
    VkCommandPool command_pool, VkQueue graphics_queue,
    Visible_Local visible_local, Vertex_Index_Buffer* vertex_index_buffer);

void uniform_buffer_create(VkDevice device, VkPhysicalDevice physical_device,
                           Buffer* uniform_buffer);

void command_pool_create(VkDevice device, u32 queue_fam_index,
                         VkCommandPool* command_pool);

void update_descritors(Region_Alloc* region, VkDevice device,
                       Descriptors* desciptors, u32 desc_count,
                       const Texture* textures, u32 num_textures,
                       Buffer* uniform_buffers);

void descriptors_create(Region_Alloc* region, VkDevice device,
                        Descriptors* desciptors, u32 desc_count,
                        VkDescriptorSetLayout desc_layout,
                        const Texture* texture, u32 num_textures,
                        Buffer* uniform_buffers);

void image_create(u32 width, u32 height, VkDevice device,
                  VkPhysicalDevice physical_device, VkFormat format,
                  VkImageTiling tiling, VkImageUsageFlags usage,
                  VkMemoryPropertyFlags wanted_mem_props,
                  VkSampleCountFlagBits num_samples, u32 mip_map_lvl,
                  VkImage* image, VkDeviceMemory* image_mem);

void image_view_create(VkDevice device, VkImage image,
                       VkImageViewType image_view_type, VkFormat image_format,
                       VkImageAspectFlags aspect_mask, u32 mip_map_lvl,
                       VkImageView* image_view);

void sampler_create(VkDevice device, Texture* textue);

void buffer_image_copy(VkDevice device, VkCommandPool command_pool, u32 width,
                       u32 height, u32 mip_map_lvl, VkBuffer src_buffer,
                       VkImage dst_image, VkQueue graphics_queue,
                       VkDeviceSize size_bytes);

void bitmap_enable(VkDevice device, VkCommandPool command_pool,
                   VkQueue graphics_queue, VkImage image,
                   const Texture* texture);

void frame_buffer_create(VkDevice device, VkRenderPass render_pass,
                         VkExtent2D extent_2D, VkImageView img_view,
                         VkImageView depth_view, VkImageView color_view,
                         VkFramebuffer* framebuffer);

u32 rand_rgb(u32 upper, u32 under);

void texture_data_set(VkDevice device, VkPhysicalDevice physical_device,
                      void* data, VkCommandPool command_pool,
                      VkQueue graphics_queue, Texture* texture,
                      VkDeviceSize size_bytes);

void image_change_layout(VkDevice device, VkCommandPool command_pool,
                         VkQueue graphic_queue, VkImage image, VkFormat format,
                         VkImageLayout old_layout, VkImageLayout new_layout);

i32 max_i(i32 f, i32 s);

void texture_path_create(VkDevice device, VkPhysicalDevice physical_device,
                         VkCommandPool command_pool, VkQueue graphics_queue,
                         b8 mip_map, VkFormat image_format,
                         const char* tex_path, Texture* texture);

u32 textures_path_create(VkDevice device, VkPhysicalDevice physical_device,
                         VkCommandPool command_pool, VkQueue graphics_queue,
                         b8 mip_map, u32 num_textures, const char** tex_paths,
                         Texture* textures);

void texture_buffer_create(VkDevice device, VkPhysicalDevice physical_device,
                           VkCommandPool command_pool, VkQueue graphics_queue,
                           VkFormat image_format, Texture* texture,
                           unsigned char* tex_buffer);

void texture_create(VkDevice device, VkPhysicalDevice physical_device,
                    u32 width, u32 height, VkCommandPool command_pool,
                    VkQueue graphics_queue, Texture* texture);

void depth_image_create(VkDevice device, VkPhysicalDevice physical_device,
                        const VkExtent2D* extent_2D,
                        VkSampleCountFlagBits sample_count, Image* depth_image);

void render_pass_begin(VkCommandBuffer command_buffer, VkRenderPass render_pass,
                       VkFramebuffer framebuffer, const VkExtent2D* extent_2D);

void render_pass_end(VkCommandBuffer command_buffer);

void push_constant(VkCommandBuffer command_buffer, VkPipelineLayout layout,
                   void* data, u32 size);

void draw(VkCommandBuffer command_buffer, u32 offset, u32 count);

void vertex_index_buffer_bind(VkCommandBuffer command_buffer,
                              const Vertex_Buffer* vert_buffer,
                              const Index_Buffer* index_buffer);

void vertex_index_buffer1_bind(VkCommandBuffer command_buffer,
                               const Vertex_Index_Buffer* buffer);

void data_buffer_copy(Buffer* buffer, void* data, size_t size_bytes);

///////// | .\Syntics\src\camera.c | //////////////////////

Camera_3D cam_3dd(void);

Camera_3D cam_3di(f32 speed, f32 sensitivity);

Camera_2D cam_2dd(void);

Camera_2D cam_2di(f32 speed, f32 sensitivity);

V2 mouse_rotation_get(const Platform* platform, f32 sens, b8* first_clicked,
                      i16* last_x, i16* last_y, f32 delta_time);

b8 camera_update(Camera_3D* camera, const Platform* platform,
                 const Events* mouse_evt, f32 delta_time, b8 off_the_ground,
                 b8 edit_mode);

void camera_print(const Camera_3D* camera);

///////// | .\Syntics\src\collision.c | //////////////////////

b8 point_in_point(V2 point_pos, V2 target, V2 target_size);

b8 point_in_aabb_2d(V2 point_pos, const AABB_2D* target);

b8 point_in_aabb_3d(V3 point_pos, const AABB_3D* target);

b8 point_in_entity_2d(V2 point_pos, const Dynamic_Entity_2D* target);

b8 rect_in_rect_normal(const Rect2D* test_obj, const Rect2D* target_obj,
                       V2* normal);

b8 rect_in_rect_2d(const Rect2D* test_obj, const Rect2D* target_obj);

b8 rect_in_rect_3d(const Rect3D* test_obj, const Rect3D* target_obj);

static void swap_f32(f32* first, f32* second);

static b8 ray_rect(V2 ray_origin, V2 ray_direction, const Rect2D* target,
                   V2* contact_point, V2* contact_normal, f32* target_hit_near);

b8 dynamic_ray_rect_unsafe(const Rect2D* test_obj, const Rect2D* target_obj,
                           V2* contact_point, V2* contact_normal,
                           f32* contact_time, f32 dt, f32 low, f32 high);

b8 dynamic_ray_rect_unsafe_d(const Rect2D* test_obj, const Rect2D* target_obj,
                             V2* contact_normal, f32 dt, f32 low, f32 high);

b8 dynamic_ray_rect(const Rect2D* test_obj, const Rect2D* target_obj,
                    V2* contact_point, V2* contact_normal, f32* contact_time,
                    f32 dt);

b8 ray_rect_rects(Rect2D* test_obj, const Rect2D* targets, u32 num_rects,
                  f32 dt);

b8 point_SAT(V2 test, Polygon2D* target);

b8 polygon2D_SAT(Polygon2D* test, Polygon2D* target);

b8 polygon2D_SAT_static(Polygon2D* test, Polygon2D* target,
                        V2* displacement_pos, V2* normal);

b8 polygon2D_lines(Polygon2D* test, Polygon2D* target);

b8 polygon2D_lines_static(Polygon2D* test, Polygon2D* target,
                          V2* displacement_pos);

///////// | .\Syntics\src\entity.c | //////////////////////

Dynamic_Entity_2D entity_2d_construct(Entity_Movement_2D* move,
                                      Entity_Misc_2D* misc);

void entity_2d_init(Region_Alloc* region, u32 max_static_entities,
                    u32 max_dynamic_entities, Entity_State_2D* entity_state);

Lookup_Key entity_dynamic_2d_add(Entity_State_2D* state);

void entity_dynamic_2d_remove(Entity_State_2D* state, Lookup_Key key);

Dynamic_Entity_2D entity_dynamic_2d_iterate(Entity_State_2D* state, u32* i);

Entity_Movement_2D* entity_movement_2d_iterate(Entity_State_2D* state, u32* i);

Entity_Movement_2D* entity_movement_2d_access(Entity_State_2D* state,
                                              Lookup_Key key);

Dynamic_Entity_2D entity_dynamic_2d_access(Entity_State_2D* state,
                                           Lookup_Key key);

Dynamic_Entity_3D entity_3d_construct(Entity_Movement_3D* move,
                                      Entity_Animation_3D* animation,
                                      Entity_Misc_3D* misc);

void entity_3d_init(Region_Alloc* region, u32 max_static_entities,
                    u32 max_dynamic_entities, Entity_State_3D* entity_state);

Lookup_Key entity_dynamic_3d_add(Entity_State_3D* state,
                                 Dynamic_Entity_3D* enity);

void entity_dynamic_3d_remove(Entity_State_3D* state, Lookup_Key key);

Dynamic_Entity_3D entity_dynamic_3d_iterate(Entity_State_3D* state, u32 i);

Entity_Movement_3D* entity_movement_3d_iterate(Entity_State_3D* state, u32 i);

Entity_Animation_3D* entity_animation_3d_iterate(Entity_State_3D* state, u32 i);

Entity_Movement_3D* entity_movement_3d_access(Entity_State_3D* state,
                                              Lookup_Key key);

Dynamic_Entity_3D entity_dynamic_3d_access(Entity_State_3D* state,
                                           Lookup_Key key);

///////// | .\Syntics\src\event_system.c | //////////////////////

void quit_event();

internal void on_key_pressed(u16 key, u16 op);

internal void on_key_released(u16 key);

void button_unpressed_set(void);

internal void on_button_pressed(u8 button);

internal void on_button_released(u8 button);

internal void on_mouse_move(i16 pos_x, i16 pos_y);

internal void on_mouse_wheel(i16 z_delta);

internal void on_window_focused(b8 focused);

internal void on_enter_leave(b8 e_l);

internal void on_window_resize(u16 width, u16 height);

void event_init(Region_Alloc* region, Platform* platform, u32 size,
                b8* running_ptr);

void event_subscribe(Events** evt, Event_Type evt_type);

void event_unsubscribe(Events** evt);

void event_poll(Platform* platform);

b8 is_key_pressed(u32 key_pressed);

b8 is_any_key_pressed(void);

internal b8 check_clicked(b8 pressed, b8* first_clicked);

b8 is_key_clicked(b8* first_clicked, u32 key_pressed);

b8 is_any_key_clicked(b8* first_clicked);

b8 is_any_button_pressed(void);

b8 is_any_button_clicked(b8* first_clicked);

b8 is_window_focused(void);

b8 is_caps_on(void);

u16 code_to_ascii(u16 key);

///////// | .\Syntics\src\file_reading.c | //////////////////////

b8 end_of_file(const File_Attrib* file);

b8 is_delim(char character, char* delims, u32 delim_len);

u32 buffer_read(File_Attrib* file, char* buffer, u32 buffer_size, char* delims,
                u32 delim_len, b8 remove_character);

u32 line_read(File_Attrib* file, char* line, u32 line_size, b8 remove_newline);

u32 statement_read(File_Attrib* file, char* buffer, u32 buffer_size,
                   b8 remove_newline);

///////// | .\Syntics\src\font.c | //////////////////////

i32* char_set(Character* chars, i32 i);

static b8 _word_get(File_Attrib* file, u32* index, char* buffer, b8* new_line);

Font font_file_load(Region_Alloc* region, const char* file_path);

V2 altas_coords_to_texidx(f32 x, f32 y, f32 atlas_width, f32 atlas_height);

u32 text_3D(Font font, const char* text, V3 pos_first_letter, f32 size,
            f32 win_width, f32 win_height, Vertex** vertices);

f32 text_x_advance(Font font, const char* text, u32 text_len, f32 size);

u32 text_2D_ttf(Font font, const char* text, V3 pos_first_letter, f32 size,
                Vertex** vertices);

u32 text_2D(Font font, f32 y_origin, const char* text, u32 text_len,
            V3 pos_first_letter, V4 color, f32 size, u32* new_lines,
            float* x_adv, Vertex_Array* vert_array);

///////// | .\Syntics\src\game.c | //////////////////////

Entity_Animation_3D dude_animation();

AABB_3D aabb_create();

AABB_Representation aabb_rep_create(AABB_3D aabb);

u32 hash_function(V3 key, u32 capacity);

Hash_Table_U32 hash_table_u32_create(Region_Alloc* region, u32 capacity,
                                     u32 collision_buffer_capacity);

Node_U32* next_node_u32(Hash_Table_U32* table);

void insert_value_u32(Hash_Table_U32* table, V3 key, u32 value);

u32* get_value_u32(Hash_Table_U32* table, V3 key);

void bubble_sort_on_y(Vertex_Array* vertices, U32_Array* indices);

void aabb_vertices_update(Vertex_Array* vertices, u32 offset, AABB_3D aabb);

void aabb_min_max_update(AABB_3D* aabb, M4 transform);

AABB_3D aabb_update(AABB_3D aabb, M4 transform, Vertex_Array* vertices,
                    u32 offset);

void aabb_check_min_max(AABB_3D* aabb, V3 pos, V3* current_max);

AABB_3D vertices_extract(const Obj_Load_Attrib* loader, f32 tex_index,
                         V3 pos_offset, Vertex_Array* vert_array,
                         U32_Array* index_array, b8 use_hash);

V3 convert_to_noise_coords(V2 x_z);

f32 noise_min_max(f32 x_offset, f32 z_offset, f32 freq, f32 grain, i32 oct,
                  f32 min, f32 max);

f32 round_down_to_half(f32 value);

void terrain_generation(f32 x_off, f32 z_off, u32 z_chunk_offset, u32 z_chunks,
                        Vertex* verts);

void generate_terrain_threaded(void* data);

void grass_generation(u32 seed, const u32 offset, const u32 iterations,
                      const u32 vertices_count, const u32 indices_count,
                      const V2* positions, const Vertex* model_vertices,
                      const u32* model_indices, Vertex* vertices, u32* indices);

void grass_generation_threaded(void* data);

void normal_generate(Vertex_Array* vert);

void game_save_binary0(const Bezier_Spline_3D* spline, V3 camera_pos);

void game_save_binary1(const Vertex_Array* vert_array,
                       const U32_Array* index_array,
                       const Bezier_Spline_3D* spline, V3 camera_pos);

void game_copy_buffer(void* data, VkCommandBuffer command_buffer,
                      u32 semaphore_idx);

void game_render(void* data, VkCommandBuffer command_buffer, u32 semaphore_idx);

void game_recreate(void* data, const Application_State* app_state);

void game_destroy(void* data, VkDevice device, u32 num_semaphores);

Bezier_Spline spline_create(Region_Alloc* region, u32 n_curves);

u32 circle_create(Vertex_Array* vert_array, u32 offset, V3 pos, f32 radius);

u32 spline_circles_curve_create(Rect3D* rects, Vertex_Array* vert_array,
                                u32 offset, Bezier_Spline_3D* spline, u32 curve,
                                f32 radius);

u32 spline_2d_circles_create(Rect3D* rects, Vertex_Array* vert_array,
                             u32 offset, Bezier_Spline* spline, f32 radius);

u32 spline_3d_circles_create(Rect3D* rects, Vertex_Array* vert_array,
                             u32 offset, Bezier_Spline_3D* spline, f32 radius);

V3 brezier_curve_pos(const Cubic_Bezier_Curve* brezier_curve, f32 t);

u32 curve_generate(Cubic_Bezier_Curve brezier_curve, Vertex_Array* vert_array,
                   u32 offset);

V3 generate_positions_curve(Bezier_Spline_3D* spline, V3 direction, V3 pos,
                            u32 side, u32 curve);

void generate_positions(Bezier_Spline_3D* spline, V3 pos);

u32 spline_generate(Bezier_Spline_3D* spline, Vertex_Array* vert_array,
                    u32 offset);

u32 normals_curve_generate(Vertex_Array* vert_array, u32 offset,
                           u32 spline_splitt, i32 half_splitt, u32 first_index,
                           u32 second_index);

void generate_spline_normals_2d(Vertex_Array* vert_array, u32 offset,
                                const Bezier_Spline* spline);

void spline_3d_normals_generate(Vertex_Array* vert_array, u32 offset,
                                const Bezier_Spline_3D* spline);

void generate_spline_curve(Bezier_Spline_3D* spline, u32 side, u32 curve,
                           Vertex_Array* vert_array_line,
                           Vertex_Array* vert_array_road);

void spline_generate_at_curve(Bezier_Spline_3D* spline, u32 side, u32 curve,
                              u32 point, V3 pos, Vertex_Array* vert_array_line,
                              Vertex_Array* vert_array_road);

void spline_generate_at_curve1(Bezier_Spline_3D* spline, u32 curve,
                               Vertex_Array* vert_array_line,
                               Vertex_Array* vert_array_road);

void generate_positions1(Bezier_Spline* spline, V3 pos);

u32 generate_spline1(Bezier_Spline* spline, Vertex_Array* vert_array,
                     u32 offset);

void generate_spline_at_curve2(Bezier_Spline* spline, u32 curve, u32 point,
                               V3 pos, Vertex_Array* vert_array_line);

f32 get_procent(Bezier_Spline sp, f32 t);

void generate_indices_terrain(U32_Array* index_array, u32 offset);

void game_update_gui(Game_State* game, Gui_Context* gui_ctx, u32 fps, f32 dt,
                     V2 dimensions);

u32 cell_index_get(V2 pos, f32 cell_size, u32 columns);

void blue_noise_2d(Region_Alloc* region, u32 seed, const u32 k, const u32 rows,
                   const u32 columns, const f32 minimum_distance,
                   V2_Array* positions);

V3 mouse_to_device_coords(V3 mouse, V2 dimensions);

void game_init(Region_Alloc* region, VkDevice device,
               VkPhysicalDevice physical_device, VkCommandPool command_pool,
               VkQueue graphic_queue, const Swap_Chain_Attrib* swap_chain,
               const Platform* platform, Render_State* render_state,
               u32 num_semaphores, Game_State* game);

void update_dudes_position(Entity_State_3D* entity_state, V3 road_pos, f32 dt);

b8 record(M4* view_matrix, f32 dt);

V3 shoot_camera_ray(VP vp, V3 mouse_device_coords);

void swap(f32* x, f32* y);

b8 ray_hit_target_aabb(V3 ray_direction, V3 ray_origin, AABB_3D target);

V3 ray_hit(V3 ray, V3 camera_pos, V3 target_pos);

void bubble_sort_rects(Rect3D* rects, u32 size);

void edit_spline(Game_State* game, V2 dimensions, b8 camera_moved, V3 ray,
                 b8 first, b8 should_update, b8* hit, b8* xyz_pressed);

f32 point_procent_along_curve_linear(Cubic_Bezier_Curve curve,
                                     V3 offset_position, V3 point_pos,
                                     f32 precision);

f32 point_procent_along_curve_binary(Cubic_Bezier_Curve curve,
                                     V3 offset_position, V3 point_pos,
                                     f32 precision);

b8 collide_with_spline(const Bezier_Spline_3D* spline, V3 offset_pos,
                       V3 test_pos, V3* collision_pos, V3* normal,
                       b8* side_collision);

void camera_move(Camera_3D* cam, V3 end_position, V3 alignment_point,
                 f32 cam_distance);

void game_update(Game_State* game, Gui_Context* gui_ctx,
                 Application_State* app_state, Frame_Data* frame, V2 dimensions,
                 u32 semaphore_idx, f32 dt);

///////// | .\Syntics\src\game_internal.c | //////////////////////

Entity_Animation_3D dude_animation();

AABB_3D aabb_create();

AABB_Representation aabb_rep_create(AABB_3D aabb);

u32 hash_function(V3 key, u32 capacity);

Hash_Table_U32 hash_table_u32_create(Region_Alloc* region, u32 capacity,
                                     u32 collision_buffer_capacity);

Node_U32* next_node_u32(Hash_Table_U32* table);

void insert_value_u32(Hash_Table_U32* table, V3 key, u32 value);

u32* get_value_u32(Hash_Table_U32* table, V3 key);

void aabb_area_init(Region_Alloc* region, u32 region_count, u32 area_count);

void bubble_sort_on_y(Vertex_Array* vertices, U32_Array* indices);

void aabb_vertices_update(Vertex_Array* vertices, u32 offset, AABB_3D aabb);

void aabb_min_max_update(AABB_3D* aabb, M4 transform);

AABB_3D aabb_update(AABB_3D aabb, M4 transform, Vertex_Array* vertices,
                    u32 offset);

void aabb_check_min_max(AABB_3D* aabb, V3 pos, V3* current_max);

AABB_3D vertices_extract(const Obj_Load_Attrib* loader, f32 tex_index,
                         V3 pos_offset, Vertex_Array* vert_array,
                         U32_Array* index_array, b8 use_hash);

V3 convert_to_noise_coords(V2 x_z);

f32 noise_min_max(f32 x_offset, f32 z_offset, f32 freq, f32 grain, i32 oct,
                  f32 min, f32 max);

f32 round_down_to_half(f32 value);

void terrain_generation(f32 x_off, f32 z_off, u32 z_chunk_offset, u32 z_chunks,
                        Vertex* verts);

void generate_terrain_threaded(void* data);

void grass_generation(u32 seed, const u32 offset, const u32 iterations,
                      const u32 vertices_count, const u32 indices_count,
                      const V2* positions, const Vertex* model_vertices,
                      const u32* model_indices, Vertex* vertices, u32* indices);

void grass_generation_threaded(void* data);

void normal_generate(Vertex_Array* vert);

void game_save_binary0(const Bezier_Spline_3D* spline, V3 camera_pos);

void game_save_binary1(const Vertex_Array* vert_array,
                       const U32_Array* index_array,
                       const Bezier_Spline_3D* spline, V3 camera_pos);

void game_copy_buffer(void* data, VkCommandBuffer command_buffer,
                      u32 semaphore_idx);

void game_render(void* data, VkCommandBuffer command_buffer, u32 semaphore_idx);

void game_recreate(void* data, const Application_State* app_state);

void game_destroy(void* data, VkDevice device, u32 num_semaphores);

Bezier_Spline spline_create(Region_Alloc* region, u32 n_curves);

u32 circle_create(Vertex_Array* vert_array, u32 offset, V3 pos, f32 radius);

u32 spline_circles_curve_create(Rect3D* rects, Vertex_Array* vert_array,
                                u32 offset, Bezier_Spline_3D* spline, u32 curve,
                                f32 radius);

u32 spline_2d_circles_create(Rect3D* rects, Vertex_Array* vert_array,
                             u32 offset, Bezier_Spline* spline, f32 radius);

u32 spline_3d_circles_create(Rect3D* rects, Vertex_Array* vert_array,
                             u32 offset, Bezier_Spline_3D* spline, f32 radius);

V3 brezier_curve_pos(const Cubic_Bezier_Curve* brezier_curve, f32 t);

u32 curve_generate(Cubic_Bezier_Curve brezier_curve, Vertex_Array* vert_array,
                   u32 offset);

V3 generate_positions_curve(Bezier_Spline_3D* spline, V3 direction, V3 pos,
                            u32 side, u32 curve);

void generate_positions(Bezier_Spline_3D* spline, V3 pos);

u32 spline_generate(Bezier_Spline_3D* spline, Vertex_Array* vert_array,
                    u32 offset);

u32 normals_curve_generate(Vertex_Array* vert_array, u32 offset,
                           u32 spline_splitt, i32 half_splitt, u32 first_index,
                           u32 second_index);

void generate_spline_normals_2d(Vertex_Array* vert_array, u32 offset,
                                const Bezier_Spline* spline);

void spline_3d_normals_generate(Vertex_Array* vert_array, u32 offset,
                                const Bezier_Spline_3D* spline);

void generate_spline_curve(Bezier_Spline_3D* spline, u32 side, u32 curve,
                           Vertex_Array* vert_array_line,
                           Vertex_Array* vert_array_road);

void spline_generate_at_curve(Bezier_Spline_3D* spline, u32 side, u32 curve,
                              u32 point, V3 pos, Vertex_Array* vert_array_line,
                              Vertex_Array* vert_array_road);

void spline_generate_at_curve1(Bezier_Spline_3D* spline, u32 curve,
                               Vertex_Array* vert_array_line,
                               Vertex_Array* vert_array_road);

void generate_positions1(Bezier_Spline* spline, V3 pos);

u32 generate_spline1(Bezier_Spline* spline, Vertex_Array* vert_array,
                     u32 offset);

void generate_spline_at_curve2(Bezier_Spline* spline, u32 curve, u32 point,
                               V3 pos, Vertex_Array* vert_array_line);

f32 get_procent(Bezier_Spline sp, f32 t);

void generate_indices_terrain(U32_Array* index_array, u32 offset);

void game_update_gui(Game_State* game, Gui_Context* gui_ctx, u32 fps, f32 dt,
                     V2 dimensions);

u32 cell_index_get(V2 pos, f32 cell_size, u32 columns);

void blue_noise_2d(Region_Alloc* region, u32 seed, const u32 k, const u32 rows,
                   const u32 columns, const f32 minimum_distance,
                   V2_Array* positions);

V3 mouse_to_device_coords(V3 mouse, V2 dimensions);

void game_init(Region_Alloc* region, VkDevice device,
               VkPhysicalDevice physical_device, VkCommandPool command_pool,
               VkQueue graphic_queue, const Swap_Chain_Attrib* swap_chain,
               const Platform* platform, Render_State* render_state,
               u32 num_semaphores, Game_State* game);

void update_dudes_position(Entity_State_3D* entity_state, V3 road_pos, f32 dt);

b8 record(M4* view_matrix, f32 dt);

V3 shoot_camera_ray(VP vp, V3 mouse_device_coords);

void swap(f32* x, f32* y);

b8 ray_hit_target_aabb(V3 ray_direction, V3 ray_origin, AABB_3D target);

V3 ray_hit(V3 ray, V3 camera_pos, V3 target_pos);

void bubble_sort_rects(Rect3D* rects, u32 size);

void edit_spline(Game_State* game, V2 dimensions, b8 camera_moved, V3 ray,
                 b8 first, b8 should_update, b8* hit, b8* xyz_pressed);

f32 point_procent_along_curve_linear(Cubic_Bezier_Curve curve,
                                     V3 offset_position, V3 point_pos,
                                     f32 precision);

f32 point_procent_along_curve_binary(Cubic_Bezier_Curve curve,
                                     V3 offset_position, V3 point_pos,
                                     f32 precision);

b8 collide_with_spline(const Bezier_Spline_3D* spline, V3 offset_pos,
                       V3 test_pos, V3* collision_pos, V3* normal,
                       b8* side_collision);

void camera_move(Camera_3D* cam, V3 end_position, V3 alignment_point,
                 f32 cam_distance);

void game_update(Game_State* game, Gui_Context* gui_ctx,
                 Application_State* app_state, Frame_Data* frame, V2 dimensions,
                 u32 semaphore_idx, f32 dt);

///////// | .\Syntics\src\gui.c | //////////////////////

b8 gui_is_focus(void);

Ui_Window ui_win(u32 id);

Gui_Context gui(void);

Hover_Clicked hover_clicked_create(const Ui_Window* win, u32 aabb_index);

u32 binary_file_parse(Gui_Context* ctx);

void binary_file_save(const Gui_Context* ctx);

void gui_frames_init(VkDevice device, VkPhysicalDevice physical_device,
                     VkCommandPool command_pool, VkQueue graphic_queue,
                     Gui_Frame* frames, u32 frame_count, u32 total_num_wins);

void gui_init(Region_Alloc* region, VkDevice device,
              VkPhysicalDevice physical_device, VkCommandPool command_pool,
              VkQueue graphic_queue, const Swap_Chain_Attrib* swap_chain,
              const Platform* platform, u32 num_semaphores, u32 total_num_wins,
              b32 use_save, Gui_Context* ctx);

void gui_draw(VkCommandBuffer command_buffer, const VkViewport* view_port,
              const VkRect2D* scissor, u32 index_offset, u32 num_indices);

void gui_copy_buffer(void* data, VkCommandBuffer command_buffer,
                     u32 semaphore_idx);

void gui_render(void* data, VkCommandBuffer command_buffer, u32 semaphore_idx);

void recreate(Region_Alloc* region);

void gui_update_begin(Gui_Context* ctx, V2 dimensions, u32 semaphore_idx,
                      f32 delta);

static void dock_blue_set(Gui_Context* ctx, u32 side_hit, V2 pos, V2 size,
                          V2 docked_pos, V2 docked_size);

void gui_update_end(Gui_Context* ctx, Gui_Frame* frame, Render_Task* copy_tasks,
                    Render_Task* render_tasks, Region_Alloc* frame_region);

void change_size(f32* win_dim_to_change, f32* pos_to_change,
                 f32* presist_offset, f32 win_dim, f32 mouse_pos);

void resize_both_set(Ui_Window* win, V2* presist_offset, V2 mouse_pos);

void resize_set(Ui_Window* win, f32* presist_offset, f32 mouse_pos);

Window_Handle window_create(Gui_Context* ctx);

void window_free(Gui_Context* ctx, Window_Handle handle);

Ui_Window* window_begin(Gui_Context* ctx, Window_Handle handle,
                        const char* title, V2 pos);

void window_end(Ui_Window** win);

void window_gridd_begin(Ui_Window* win, u32 x, u32 y);

void window_gridd_end(Ui_Window* win);

static void set_biggest_wide(Ui_Window* win);

static void misc_update(Ui_Window* win);

static f32 calculate_text_advance(const Font* font, const char* buffer,
                                  u32 len);

static V4 hand_hover(const Ui_Window* win, V4 color, b32 hover, b8 ui_hold);

b8 window_button_add(Ui_Window* win, const char* text);

static b8 is_character_number(u16 key);

static b8 is_character_letter(u16 key);

static b8 _input_focused(Ui_Window* win, Input* curr_input, char* text,
                         char* last_text, u32 text_size, b8 clicked,
                         b8 allow_letters, b8 cache_on_leave);

u32 _render_input(Ui_Window* win, Input* curr_input, const char* text,
                  V4 input_color, V4 text_color, f32 min);

b8 window_input_float_add(Ui_Window* win, f32* input, f32 min, f32 max,
                          f32 speed);

b8 window_text_input_add(Ui_Window* win, char* ptr_to_text, u32* size);

void window_text_add(Ui_Window* win, const char* text);

static u32 buffer_flush(void** s_buffer, u32 size_bytes, f32 multiplier);

static void terminal_flush(Terminal_Attrib* term);

void terminal_add(Gui_Context* ctx, Terminal_Attrib* term, Ui_Window* win,
                  f32 width, f32 height);

void gui_destroy(Gui_Context* ctx, VkDevice device, u32 num_semaphores);

void sy_print_text(Terminal_Attrib* term, char* text);

///////// | .\Syntics\src\instance_device.c | //////////////////////

VKAPI_ATTR VkBool32 VKAPI_CALL msg_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

VkDebugUtilsMessengerCreateInfoEXT config_debug_info(void);

void debug_messenger_init(Instance_State* state);

void debug_messenger_destroy(VkInstance instance,
                             VkDebugUtilsMessengerEXT debugMessenger,
                             const VkAllocationCallbacks* pAllocator);

void instance_init(VkInstance* instance);

Queue_Family_Indices queue_indices_get(Region_Alloc* region,
                                       VkPhysicalDevice physical_device,
                                       VkSurfaceKHR surface, b8* all_supported);

void physical_device_pick(Region_Alloc* region, VkInstance instance,
                          VkSurfaceKHR surface,
                          VkPhysicalDevice* physical_device,
                          Queue_Family_Indices* q_indices);

void logical_device_create(VkPhysicalDevice physical_device,
                           Queue_Family_Indices q_indices, VkDevice* device);

void instance_destroy(Instance_State* state);

///////// | .\Syntics\src\linux\linux_platform.c | //////////////////////

Mutex mutex_create();

void mutex_lock(Mutex* mutex);

void mutex_unlock(Mutex* mutex);

void mutex_destroy(Mutex* mutex);

Semaphore semaphore_create(i32 initial_count, i32 max_count);

void semaphore_wait_and_decrement(Semaphore* sem);

void semaphore_increment(Semaphore* sem);

void semaphore_destroy(Semaphore* sem);

Thread_Handle thread_create(void* data,
                            thread_return_value (*thread_function)(void* data),
                            unsigned long creation_flag,
                            unsigned long* thread_id);

void thread_join(Thread_Handle handle);

void thread_destroy(Thread_Handle handle);

u32 platform_core_count();

void platform_title_change(Platform* platform, const char* title, u32 len);

xcb_connection_t* platform_connection_get(Platform* platform);

xcb_window_t platform_window_get(Platform* platform);

void platform_init(Region_Alloc* region, const char* title, u16* width,
                   u16* height, b32 full_screen, Platform** platform);

void platform_event_set_callbacks(
    Platform* platform, void (*on_key_pressed)(u16 key, u16 op),
    void (*on_key_released)(u16 key), void (*on_button_pressed)(u8 key),
    void (*on_button_released)(u8 key),
    void (*on_mouse_move)(i16 pos_x, i16 pos_y),
    void (*on_mouse_wheel)(i16 z_delta), void (*on_window_focused)(b8 focused),
    void (*on_enter_leave)(b8 e_l),
    void (*on_window_resize)(u16 width, u16 height));

void event_fire(Platform* platform);

void move_main_window(Platform* platform);

void platform_window_get_size(const Platform* platform, u16* width,
                              u16* height);

void platform_cursor_hide(const Platform* platform);

void platform_cursor_show(const Platform* platform);

void platform_mouse_set_pos(const Platform* platform, i16 pos_x, i16 pos_y);

void platform_cursor_show_centered(const Platform* platform);

void platform_mouse_set_last_pos(const Platform* platform);

void platform_cursor_show_last_pos(const Platform* platform);

void platform_cursor_change(const Platform* platform, u32 cursor_id);

void platform_mouse_get_pos(i16* pos_x, i16* pos_y);

f64 platform_get_time();

void platform_sleep(u64 milli);

void platform_shut_down(Platform* platform);

void file_read(File_Attrib* file_attrib, Region_Alloc* region,
               const char* file_path);

void file_write(const char* file_path, const char* mode, const char* content,
                u32 size);

void file_write_append_end(const char* file_path, const char* content);

void file_write_entire(const char* file_path, const char* content, u32 size);

u32 executable_directory(char* file, u32 size);

///////// | .\Syntics\src\logging.c | //////////////////////

void logging_init(Region_Alloc* region);

u32 terminal_buffer_size_get();

Terminal_Attrib* terminal_ptr_get();

void set_log(b8 set_val);

b8 use_log(void);

void set_log_alloc(b8 set_val);

b8 use_log_alloc(void);

char* line_file_to_buffer(const char* file, i32 line, const char* msg);

void _ERROR(const char* file, i32 line, const char* msg);

void sy_print(const char* format, ...);

///////// | .\Syntics\src\lookup_table.c | //////////////////////

Lookup_Table lookup_table_create(Region_Alloc* region, u32 n_entries);

Lookup_Key entry_add(Lookup_Table* table, u32 ref_index);

u32 table_index(Lookup_Table* table, Lookup_Key key);

u32 entry_remove(Lookup_Table* table, Lookup_Key key);

void entry_index_change(Lookup_Table* table, u32 entry, u32 new_index);

///////// | .\Syntics\src\main.c | //////////////////////
}
#else

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line,
                   int show_cmd);

///////// | .\Syntics\src\math\syntics_math.c | //////////////////////

V2_Array v2_array_create(Region_Alloc* region, u32 capacity);

u32 v2_array_push(V2_Array* array, V2 data);

V2* v2_array_val_ptr(V2_Array* array, u32 index);

V2 v2_array_pop(V2_Array* array);

V3_Array v3_array_create(Region_Alloc* region, u32 capacity);

u32 v3_array_push(V3_Array* array, V3 data);

V3* v3_array_val_ptr(V3_Array* array, u32 index);

V3 v3_array_pop(V3_Array* array);

Vertex_Array vertex_array_create(Region_Alloc* region, u32 capacity);

Vertex_Array vertex_array_ref_at_size_offset(Vertex_Array* array,
                                             u32 ref_capacity);

u32 vertex_array_push(Vertex_Array* array, Vertex data);

Vertex* vertex_array_val_ptr(Vertex_Array* array, u32 index);

Vertex vertex_array_pop(Vertex_Array* array);

U32_Array u32_array_create(Region_Alloc* region, u32 capacity);

U32_Array u32_array_ref_at_size_offset(U32_Array* array, u32 ref_capacity);

u32 u32_array_push(U32_Array* array, u32 data);

u32 u32_array_pop(U32_Array* array);

u32* u32_array_val_ptr(U32_Array* array, u32 index);

u32* u32_array_back(U32_Array* array);

V2 v2d(void);

V2 v2i(f32 i);

V2 v2f(f32 x, f32 y);

V2 v2_v3(V3 v3);

V2 v2_v4(V4 v4);

V3 v3d(void);

V3 v3i(f32 i);

V3 v3f(f32 x, f32 y, f32 z);

V2 v2_random(u32 seed, f32 min, f32 max);

V3 v3_random(u32 seed, f32 min, f32 max);

V3 v3_v2(V2 v2);

V3 v3_v2f(V2 v2, f32 z);

V3 v3_v4(V4 v4);

V4 v4d(void);

V4 v4i(f32 i);

V4 v4ic(f32 i);

V4 v4f(f32 x, f32 y, f32 z, f32 w);

V4 v4_v2(V2 v2);

V4 v4_v2f(V2 v2, f32 z, f32 w);

V4 v4_v3(V3 v3);

V4 v4_v3f(V3 v3, f32 w);

f32 v2_sum(V2 v);

f32 v3_sum(V3 v);

f32 v4_sum(V4 v);

V2 v2_add(V2 v1, V2 v2);

V3 v3_add(V3 v1, V3 v2);

V4 v4_add(V4 v1, V4 v2);

V2 v2_sub(V2 v1, V2 v2);

V3 v3_sub(V3 v1, V3 v2);

V4 v4_sub(V4 v1, V4 v2);

V2 v2_s_add(V2 v1, f32 s);

V3 v3_s_add(V3 v1, f32 s);

V4 v4_s_add(V4 v1, f32 s);

V2 v2_s_sub(V2 v1, f32 s);

V3 v3_s_sub(V3 v1, f32 s);

V4 v4_s_sub(V4 v1, f32 s);

V2 v2_s_multi(V2 v1, f32 s);

V3 v3_s_multi(V3 v1, f32 s);

V4 v4_s_multi(V4 v1, f32 s);

V2 v2_neg(V2 v);

V3 v3_neg(V3 v);

V4 v4_neg(V4 v);

V2 v2_multi(V2 v1, V2 v2);

V3 v3_multi(V3 v1, V3 v2);

V4 v4_multi(V4 v1, V4 v2);

V2 v2_s_div(V2 v1, f32 s);

V3 v3_s_div(V3 v1, f32 s);

V4 v4_s_div(V4 v1, f32 s);

void v2_add_equal(V2* v1, V2 v2);

void v3_add_equal(V3* v1, V3 v2);

void v4_add_equal(V4* v1, V4 v2);

void v2_sub_equal(V2* v1, V2 v2);

void v3_sub_equal(V3* v1, V3 v2);

void v4_sub_equal(V4* v1, V4 v2);

void v2_s_add_equal(V2* v1, f32 s);

void v3_s_add_equal(V3* v1, f32 s);

void v4_s_add_equal(V4* v1, f32 s);

void v2_s_sub_equal(V2* v1, f32 s);

void v3_s_sub_equal(V3* v1, f32 s);

void v4_s_sub_equal(V4* v1, f32 s);

void v2_s_multi_equal(V2* v1, f32 s);

void v3_s_multi_equal(V3* v1, f32 s);

void v4_s_multi_equal(V4* v1, f32 s);

void v2_s_div_equal(V2* v1, f32 s);

void v3_s_div_equal(V3* v1, f32 s);

void v4_s_div_equal(V4* v1, f32 s);

b8 v2_equal(V2 v1, V2 v2);

b8 v3_equal(V3 v1, V3 v2);

b8 v4_equal(V4 v1, V4 v2);

b8 v2_less(V2 v1, V2 v2);

b8 v3_less(V3 v1, V3 v2);

b8 v4_less(V4 v1, V4 v2);

b8 v2_more(V2 v1, V2 v2);

b8 v3_more(V3 v1, V3 v2);

b8 v4_more(V4 v1, V4 v2);

P2 p2d(void);

P2 p2i(f32 i);

P2 p2f(f32 x, f32 y);

P2 p2_p3(P3 p3);

P2 p2_p4(P4 p4);

P3 p3d(void);

P3 p3i(f32 i);

P3 p3f(f32 x, f32 y, f32 z);

P3 p3_p2(P2 p2);

P3 v3_p2f(P2 p2, f32 z);

P3 p3_p4(P4 p4);

P4 p4d(void);

P4 p4i(f32 i);

P4 p4f(f32 x, f32 y, f32 z, f32 w);

P4 p4_p2(P2 p2);

P4 p4_p2f(P2 p2, f32 z, f32 w);

P4 p4_p3(P3 p3);

f32 p2_sum(P2 p);

f32 p3_sum(P3 p);

f32 p4_sum(P4 p);

P2 p2_add(P2 p1, P2 p2);

P3 p3_add(P3 p1, P3 p2);

P4 p4_add(P4 p1, P4 p2);

V2 p2_sub(P2 p1, P2 p2);

V3 p3_sub(P3 p1, P3 p2);

V4 p4_sub(P4 p1, P4 p2);

P2 p2_s_multi(P2 p1, f32 s);

P3 p3_s_multi(P3 p1, f32 s);

P4 p4_s_multi(P4 p1, f32 s);

M2 m2i(f32 i);

M2 m2d(void);

M3 m3i(f32 i);

M3 m3d(void);

M3 m3f(f32 f0, f32 f1, f32 f2, f32 f3, f32 f4, f32 f5, f32 f6, f32 f7, f32 f8);

M3 m3_m4(M4 matrix);

M4 m4i(f32 i);

M4 m4d(void);

M4 m4f(f32 f0, f32 f1, f32 f2, f32 f3, f32 f4, f32 f5, f32 f6, f32 f7, f32 f8,
       f32 f9, f32 f10, f32 f11, f32 f12, f32 f13, f32 f14, f32 f15);

M4 m4_v4(V4 c0, V4 c1, V4 c2, V4 c3);

f32 m2_sum(M2 m);

f32 m3_sum(M3 m);

f32 m4_sum(M4 m);

M2 m2_add(M2 m1, M2 m2);

M3 m3_add(M3 m1, M3 m2);

M4 m4_add(M4 m1, M4 m2);

M2 m2_sub(M2 m1, M2 m2);

M3 m3_sub(M3 m1, M3 m2);

M4 m4_sub(M4 m1, M4 m2);

M2 m2_s_multi(M2 m, f32 s);

M3 m3_s_multi(M3 m, f32 s);

M4 m4_s_multi(M4 m, f32 s);

V2 m2_v2_multi(M2 m, V2 v);

V3 m3_v3_multi(M3 m, V3 v);

V3 m4_v3_multi(M4 m, V3 v);

V4 m4_v4_multi(M4 m, V4 v);

M2 m2_multi(M2 m1, M2 m2);

M3 m3_multi(M3 m1, M3 m2);

M4 m4_multi(M4 m1, M4 m2);

M4 m4_s_div(M4 m, f32 s);

b8 m2_equal(M2 m1, M2 m2);

b8 m3_equal(M3 m1, M3 m2);

b8 m4_equal(M4 m1, M4 m2);

b8 m2_less(M2 m1, M2 m2);

b8 m3_less(M3 m1, M3 m2);

b8 m4_less(M4 m1, M4 m2);

b8 m2_more(M2 m1, M2 m2);

b8 m3_more(M3 m1, M3 m2);

b8 m4_more(M4 m1, M4 m2);

b8 vertex_equal(const Vertex* f, const Vertex* s);

b8 vp_equal(const VP* f, const VP* s);

Vertex vertex_create(V3 pos, V3 normal, V2 tex_coords, V4 color, f32 tex_index);

Polygon2D poly2D(V2 pos, V2* p_arr, V2* n_arr, u32 n_sides);

f32 abs_f32(f32 in);

f32 clampf32(f32 value, f32 min, f32 max);

f32 clampf32_low(f32 value, f32 min);

V4 clamp(V4 v1, V4 min, V4 max);

f32 minf32(f32 f1, f32 f2);

f32 maxf32(f32 f1, f32 f2);

f32 v2_len(V2 v2);

f32 v2_len_squared(V2 v2);

f32 v3_len_squared(V3 v3);

f32 v3_len(V3 v3);

V3 v3_lerp(V3 v1, V3 v2, f32 t);

f32 v2_dot(V2 v1, V2 v2);

f32 v3_dot(V3 v1, V3 v2);

f32 v3_angle(V3 v1, V3 v2);

V2 v2_normalize(V2 v2);

V3 v3_normalize(V3 v3);

f32 v2_cross(V2 v1, V2 v2);

V3 v3_cross(V3 v1, V3 v2);

V3 v3_project(V3 v1, V3 v2);

V3 v3_reject(V3 v1, V3 v2);

f32 v2_distance(V2 v1, V2 v2);

f32 v3_distance_squared(V3 v1, V3 v2);

f32 v3_distance(V3 v1, V3 v2);

f32 p3_distance(P3 p1, P3 p2);

f32 p3_distance_sqrt(P3 p1, P3 p2);

P3 p3_lerp(P3 p1, P3 p2, f32 t);

P3 p3_min(P3 p1, P3 p2);

P3 p3_max(P3 p1, P3 p2);

P3 p3_abs(P3 p);

f32 radians(f32 deg);

f32 m3_determinant(M3 m3);

f32 m4_determinant(M4 m4);

M3 m3_transpose(M3 m3);

M4 m4_transpose(M4 m4);

M3 m3_rotate(M3 m3, f32 rad);

M4 rotate_x(f32 rad);

M4 rotate_y(f32 rad);

M4 rotate_z(f32 rad);

V3 v3_rotate(V3 v3, f32 rad, V3 normal);

M3 translate(V2 v);

M4 m4_translate(V3 v3);

M3 scale(V2 v);

M4 m4_scale(V3 v);

M4 m4_shear(V3 v, V2 hx, V2 hy, V2 hz);

M4 ortho(f32 left, f32 right, f32 bottom, f32 top, f32 sy_near, f32 sy_far);

M4 view(V3 eye, V3 center, V3 up);

M4 perspective(f32 fov, f32 aspect, f32 sy_near, f32 sy_far);

M4 inverse(M4 m);

b8 is_poly2d_convex(Polygon2D p);

Plane plane(P3 a, P3 b, P3 c);

///////// | .\Syntics\src\noise.c | //////////////////////

f32 sy_normalize_f32(f32 value, f32 min, f32 max);

f32 sy_fade(f32 t);

f32 sy_lerp(f32 a, f32 b, f32 t);

i32 sy_noise2(i32 x, i32 y);

f32 sy_smooth_inter(f32 a, f32 b, f32 t);

f32 sy_noise2d(f32 x, f32 y);

f32 sy_value_noise2d(f32 x, f32 y, f32 freq, f32 gain, i32 oct);

///////// | .\Syntics\src\notebook.c | //////////////////////

void notebook_init(Region_Alloc* region, VkDevice device,
                   VkPhysicalDevice physical_device, VkCommandPool command_pool,
                   VkQueue graphic_queue, const Swap_Chain_Attrib* swap_chain,
                   const Platform* platform, Render_State* render_state,
                   u32 num_semaphores, Notebook* notebook);

void notebook_render(void* data, VkCommandBuffer command_buffer,
                     u32 semaphore_idx);

void notebook_update_gui(Notebook* note, Gui_Context* gui_ctx, f32 dt,
                         V2 dimensions);

void notebook_update(Notebook* note, Gui_Context* gui_ctx,
                     Application_State* app_state, V2 dimensions,
                     u32 semaphore_idx, f32 dt);

///////// | .\Syntics\src\notebook_app.c | //////////////////////

void run_notebook_app(void);

///////// | .\Syntics\src\obj_load.c | //////////////////////

internal void _init(u32 v, u32 vn, u32 vt, u32 f, Obj_Load_Attrib* obj_attrib);

internal void parse_sizes(File_Attrib* file, u32* v, u32* vt, u32* vn, u32* f);

internal V3 vec3f(const char* line);

internal V2 vec2f(const char* line);

internal void _f_parse(Obj_Load_Attrib* obj_attrib, char* line);

internal void _buffer_parse(Obj_Load_Attrib* obj_attrib, File_Attrib* file);

void model_load(Obj_Load_Attrib* obj_attrib, const char* model_path);

void obj_load_free(Obj_Load_Attrib* obj_load);

///////// | .\Syntics\src\random.c | //////////////////////

void set_seed(void);

u32 random_uint(u32 low, u32 high);

f32 random_f32(f32 low, f32 high);

u32 random_u32s(u32 seed);

u32 random_u32ss(u32 seed, u32 low, u32 high);

f32 random_f32s(u32 seed, f32 low, f32 high);

///////// | .\Syntics\src\region_alloc.c | //////////////////////

Array_Head array_head_create(u32 capacity, u32 size);

b8 region_init(Region_Alloc* region, u64 size);

void stack_init(u32 size);

Region_Alloc* _stack_get(u32 check_val);

u64 stack_size();

void stack_reset(void);

u64 _stack_begin_scope(void);

void _stack_end_scope(u64 size_at_start);

u32 alignment_offset_get(u8* current_pos, u32 alignment);

static void* malloc_init(Region_Alloc* region, u32 size, u32 alignment);

void* _region_malloc(Region_Alloc* region, u32 size, u32 alignment);

void* _region_calloc(Region_Alloc* region, u32 size, u32 alignment);

void _region_pop(Region_Alloc* region, u32 size, Allocation_Type alloc_type);

void region_reset(Region_Alloc* region);

void region_free(Region_Alloc* region);

void region_print(const Region_Alloc* region);

static void* array_init(Region_Alloc* region, u32 capacity, u32 type,
                        u32 alignment);

void* _region_array(Region_Alloc* region, u32 capacity, u32 type,
                    u32 alignment);

void* _region_array_calloc(Region_Alloc* region, u32 capacity, u32 type,
                           u32 alignment);

void* _region_array_val(Region_Alloc* region, u32 capacity, u32 type,
                        u32 alignment, const void* values);

Array_Head* _array_check(void* array);

b8 _array_check_size(void* array);

u32 _array_check_size_index(void* array, u32 index);

u32 _array_check_pop_size(void* array);

void _array_clear(void* array, u32 stride);

u32 array_size(const void* const array);

u32 array_capacity(const void* const array);

char* path_extend(Region_Alloc* region, const char* trailing_path,
                  u32 trailing_path_len);

///////// | .\Syntics\src\render.c | //////////////////////

u32 semaphore_idx_get(Render_State* render_state);

void fence_semaphore_create(VkDevice device, VkFence* fence,
                            VkSemaphore* image_semaphores,
                            VkSemaphore* present_semaphores);

void render_state_init(Region_Alloc* region, VkDevice device, Queues queues,
                       VkPhysicalDevice physical_device,
                       VkCommandPool command_pool,
                       const Queue_Family_Indices* q_indices,
                       u32 num_semaphores, const Swap_Chain_Attrib* swap_chain,
                       const Platform* platform, Render_State** render_state);

VkQueue graphic_queue_get(Render_State* render_state);

void render_callback(Render_State* render_state,
                     void (*draw_callback)(void* data,
                                           VkCommandBuffer command_buffer,
                                           u32 semaphore_idx),
                     void* data);

void subscribe_update_callback(
    Render_State* render_state,
    void (*update_callback_p)(void* data, Region_Alloc* region,
                              const Application_State* app_state,
                              Render_State* render_state, V2 dimensions,
                              u32 semaphore_idx, f32 dt),
    void* data);

void subscribe_recreate_callback(
    Render_State* render_state,
    void (*rc_callback)(void* data, const Application_State* app_state),
    void* data);

void subscribe_recreate_gp_callback(
    Render_State* render_state,
    void (*rc_gp_callback)(void* data, const Application_State* app_state),
    void* data);

void subscribe_destroy_callback(Render_State* render_state,
                                void (*destroy_callback)(void* data,
                                                         VkDevice device,
                                                         u32 num_semaphores),
                                void* data);

void submit_and_present(VkQueue graphic_queue, VkQueue present_queue,
                        VkSemaphore image_semaphore,
                        VkSemaphore present_semaphore, VkFence fence,
                        VkCommandBuffer* command_buffers,
                        u32 command_buffer_count, VkSwapchainKHR swap_chain,
                        u32 image_index);

void frame_begin(Render_State* render_state, Application_State* app_state);

void frame_render(Render_State* render_state, Application_State* app_state,
                  Render_Task* copy_tasks, Render_Task* render_tasks, f32 dt);

void render_state_destroy(VkDevice device, Render_State* render_state);

///////// | .\Syntics\src\render_util.c | //////////////////////

AABB_2D _set_up_verticies(Vertex_Array* vert_array, u32* rect_count, V3 pos,
                          V2 size, V4 color, f32 tex_index,
                          Tex_Coords tex_coords);

AABB_2D quad(Vertex_Array* vert_array, u32* rect_count, V3 pos, V2 size,
             V4 color, f32 tex_index);

AABB_2D quad_f(Vertex_Array* vert_array, u32* rect_count, V3 pos, V2 size,
               V4 color, f32 tex_index);

AABB_2D quad_gradiant_l_r(Vertex_Array* vert_array, u32* rect_count, V3 pos,
                          V2 size, V4 left_color, V4 right_color,
                          f32 tex_index);

AABB_2D quad_gradiant_t_b(Vertex_Array* vert_array, u32* rect_count, V3 pos,
                          V2 size, V4 top_color, V4 bottom_color,
                          f32 tex_index);

AABB_2D quad_s_gradiant_l_r(Vertex_Array* vert_array, u32* rect_count, V3 pos,
                            V2 size, V4 left_color, V4 right_color,
                            f32 tex_index, f32 shadow_offset);

AABB_2D quad_s_gradiant_t_b(Vertex_Array* vert_array, u32* rect_count, V3 pos,
                            V2 size, V4 top_color, V4 bottom_color,
                            f32 tex_index, f32 shadow_offset);

AABB_2D quad_s_gradiant(Vertex_Array* vert_array, u32* rect_count, V3 pos,
                        V2 size, V4 color, f32 multiplier, f32 tex_index,
                        f32 shadow_offset);

AABB_2D quad_s(Vertex_Array* vert_array, u32* rect_count, V3 pos, V2 size,
               V4 color, f32 tex_index, f32 shadow_offset);

AABB_2D quad_sl(Vertex_Array* vert_array, u32* rect_count, V3 pos, V2 size,
                V4 color, f32 tex_index, f32 shadow_offset);

AABB_2D quad_sl_gradiant(Vertex_Array* vert_array, u32* rect_count, V3 pos,
                         V2 size, V4 color, f32 tex_index, f32 shadow_offset);

AABB_2D border_add_s(Vertex_Array* vert_array, u32* num_indices,
                     V4 border_color, V3 top_left, V2 size, f32 thickness,
                     f32 tex_index);

AABB_2D border_add(Vertex_Array* vert_array, u32* num_indices, V4 border_color,
                   V3 top_left, V2 size, f32 thickness, f32 tex_index);

void quad_middle(Vertex_Array* vert_array, V3 pos, V2 size, V4 color,
                 f32 tex_index);

void polygon2D_draw_quads(Vertex_Array* vert_array, Polygon2D poly, f32 z,
                          V4 color, f32 line_width, f32 tex_index);

void polygon2D_draw_lines(Vertex_Array* vert_array, U32_Array* idx_array,
                          Polygon2D poly, f32 z, V4 color, f32 tex_index);

internal void indices_insert(U32_Array* idx_array, u32 p_i, u32 added_val0,
                             u32 added_val1);

void square_rounded_corners(Vertex_Array* vert_array, U32_Array* idx_array,
                            u32 vertex_offset, V3 pos, V2 size, V4 color,
                            f32 seperation, u32 corner_vertices_count,
                            f32 tex_index, u32 index_index);

void square_rounded_corners_3d(Vertex_Array* vert_array, U32_Array* idx_array,
                               const u32 vertex_offset, V3 pos, V3 size,
                               V4 color, f32 seperation,
                               u32 corner_vertices_count, f32 tex_index);

void indices_generate(U32_Array* array, u32 offset, u32 indices_count);

u32 cube(Vertex_Array* vert_array, u32 offset, V3 pos, V3 size, V4 color,
         f32 tex_index);

void cube1(Vertex_Array* vert_array, V3 pos, V3 size, V4 color, f32 tex_index);

void cube_not_center1(Vertex_Array* vert_array, V3 pos, V3 size, V4 color,
                      f32 tex_index);

u32 cube_not_center(Vertex_Array* vert_array, u32 offset, V3 pos, V3 size,
                    V4 color, f32 tex_index);

void cube_indices_offset(U32_Array* indices, u32 offset, u32 how_many);

void cube_indices(U32_Array* indices, u32 offset, u32 how_many);

u32 gridd_using_line_list(Vertex_Array* vert_array, u32 vertex_offset,
                          U32_Array* indices, u32 index_offset, V3 middle_pos,
                          V2 spacing, u32 lines_width_count,
                          u32 lines_height_count, V4 color, f32 tex_index);

///////// | .\Syntics\src\simple_particle.c | //////////////////////

void particles_2d_init(Region_Alloc* region, Particles_2D* particles,
                       u32 max_particles);

void particle_2d_emit(Particles_2D* particles,
                      const Particle_Attrib_2D* particle_attrib,
                      V2 individual_speed, V2 neg_alt, f32 life);

u32 particles_2d_update(Particles_2D* particles, Vertex_Array* vertices,
                        f32 dt);

void particles_3d_init(Region_Alloc* region, Particles_3D* particles,
                       u32 max_particles);

void particles_3d_reset(Particles_3D* particles);

void particle_3d_emit(Particles_3D* particles,
                      const Particle_Attrib_3D* particle_attrib,
                      V3 individual_speed, V3 neg_alt, f32 random, f32 life);

u32 particles_3d_update(Particles_3D* particles, Vertex_Array* vertices,
                        u32 vertex_offset, f32 dt);

///////// | .\Syntics\src\swap_chain.c | //////////////////////

u32 u32_clamp(u32 value, u32 min, u32 max);

VkSampleCountFlagBits max_usable_sample_count(VkPhysicalDevice physical_device);

void swapchain_create(VkPhysicalDevice physical_device, VkDevice device,
                      VkSurfaceKHR surface, u32 width, u32 height,
                      Queue_Family_Indices indices,
                      VkSwapchainKHR old_swap_chain, b8 vsync,
                      Swap_Chain_Attrib* swap_chain);

void render_pass_create(VkDevice device, VkFormat color_format,
                        VkSampleCountFlagBits sample_count,
                        VkRenderPass* render_pass);

void swapchain_images_get(Region_Alloc* region, VkDevice device,
                          Swap_Chain_Attrib* swap_chain);

Vertex_Info vertex_get_info();

void descriptor_set_layout_create(VkDevice device, u32 num_textures,
                                  VkDescriptorSetLayout* layout);

void pipeline_layout_create(VkDevice device, VkDescriptorSetLayout set_layout,
                            VkPipelineLayout* layout);

void graphics_pipeline_create(VkDevice device, VkRenderPass render_pass,
                              VkSampleCountFlagBits sample_count,
                              VkPipelineLayout pipeline_layout,
                              const Vertex_Info* vertex_info,
                              Graphic_Pipeline_Attrib* graphic_info,
                              const char* vert_path, const char* frag_path,
                              VkPipeline* graphic_pipline);

void uniforms_descriptors_init(Region_Alloc* region, VkDevice device,
                               VkPhysicalDevice physical_device,
                               Buffer** uniform_buffers,
                               Descriptors* descriptors,
                               VkDescriptorSetLayout set_layout,
                               u32 num_semaphores, const Texture* textures,
                               u32 num_textures);

void graphics_pipeline_create_deluxe(VkDevice device,
                                     VkPipelineLayout pipeline_layout,
                                     Graphic_Pipeline_Attrib* graphic_info,
                                     const char* vert_path,
                                     const char* frag_path,
                                     const Swap_Chain_Attrib* swap_chain,
                                     VkPipeline* graphic_pipline);

void multisample_enable(const Swap_Chain_Attrib* swap_chain, VkDevice device,
                        VkPhysicalDevice physical_device, Image* color_image);

void graphic_pipline_recreate(VkDevice device, VkPipelineLayout pipeline_layout,
                              Graphic_Pipeline_Attrib* graphic_info,
                              const char* vert_path, const char* frag_path,
                              const Swap_Chain_Attrib* swap_chain,
                              VkPipeline* graphic_pipline);

void swapchain_recreate(Application_State* app_state, u32 width, u32 height);

///////// | .\Syntics\src\syntics.c | //////////////////////

void find_working_dir(Region_Alloc* region);

///////// | .\Syntics\src\syntic_app.c | //////////////////////

void instance_init_threaded(void* data);

void game_logic(void* data);

void render_logic(void* data);

void run_app(void);

///////// | .\Syntics\src\test_bed.c | //////////////////////

internal void test_bed_destroy(void* data, VkDevice device, u32 num_semaphores);

void test_bed_render(void* data, VkCommandBuffer command_buffer,
                     u32 semaphore_idx);

u32 circle(Vertex* vertices, u32 vertex_offset, u32* indices, u32 index_offset,
           u32* indices_count, V3 middle_pos, u32 triangle_count, f32 radius,
           V4 color, f32 tex_index);

void test_bed_recreate_gps(void* data, const Application_State* app_state);

void test_bed_init(Region_Alloc* region, VkDevice device,
                   VkPhysicalDevice physical_device, VkCommandPool command_pool,
                   VkQueue graphic_queue, const Swap_Chain_Attrib* swap_chain,
                   const Platform* platform, Render_State* render_state,
                   u32 num_semaphores);

void test_update_gui(Region_Alloc* region, const Application_State* app_state,
                     f32 dt, V2 dimensions);

V4 animate_colors(f32 dt);

void color_change(u32 id, V4 new_color);

void test_bed_new_game(u32 id, b8 any_button_clicked, f32 dt);

void test_bed_saved_game(u32 id, b8 any_button_clicked, f32 dt);

void test_bed_settings(u32 id, b8 any_button_clicked, f32 dt);

void test_bed_quit(u32 id, b8 any_button_clicked, f32 dt);

void test_bed_process_options(u32 id, b8 any_button_clicked, f32 dt);

void test_bed_update(Region_Alloc* region, Frame_Data* frame,
                     const Application_State* app_state,
                     Render_State* render_state, V2 dimensions,
                     u32 semaphore_idx, f32 dt);

///////// | .\Syntics\src\thread_queue.c | //////////////////////

Thread_Task thread_task(void (*task_callback)(void* data), void* data);

void semaphore_counter_wait(Semaphore_Counter* semaphore_counter);

void semaphore_counter_wait_and_free(Semaphore_Counter* semaphore_counter);

void _thread_task_push(Thread_Task task, Semaphore* semaphore);

void thread_tasks_push(Thread_Task* tasks, u32 task_count,
                       Semaphore_Counter* semaphore_counter);

Thread_Task_Internal thread_task_pop();

thread_return_value thread_loop(void* data);

void thread_init(Region_Alloc* region, u32 capacity, u32 thread_count);

void threads_destroy();

///////// | .\Syntics\src\vulkan_api.c | //////////////////////

void vulkan_init(Region_Alloc* region, Instance_State* instance_state,
                 Application_State* app_state, Render_State** render_state,
                 u32 width, u32 height);

void vulkan_destroy(Application_State* app_state, Render_State* render_state);

///////// | .\Syntics\src\win32\win32_platform.c | //////////////////////

Mutex mutex_create();

void mutex_lock(Mutex* mutex);

void mutex_unlock(Mutex* mutex);

void mutex_destroy(Mutex* mutex);

Semaphore semaphore_create(i32 initial_count, i32 max_count);

void semaphore_wait_and_decrement(Semaphore* sem);

void semaphore_increment(Semaphore* sem);

void semaphore_destroy(Semaphore* sem);

Thread_Handle thread_create(void* data,
                            thread_return_value (*thread_function)(void* data),
                            unsigned long creation_flag,
                            unsigned long* thread_id);

void thread_join(Thread_Handle handle);

void thread_destroy(Thread_Handle handle);

u32 platform_core_count();

void error_msg(const char* msg);

HWND platform_window_get(Platform* platform);

LRESULT msg_handler(HWND win, UINT msg, WPARAM w_param, LPARAM l_param);

static void sy_fullscreen(HWND window);

void platform_init(Region_Alloc* region, const char* title, u16* width,
                   u16* height, b32 full_screen, Platform** platform);

void platform_event_set_callbacks(
    Platform* platform, void (*on_key_pressed)(u16 key, u16 op),
    void (*on_key_released)(u16 key), void (*on_button_pressed)(u8 key),
    void (*on_button_released)(u8 key),
    void (*on_mouse_move)(i16 pos_x, i16 pos_y),
    void (*on_mouse_wheel)(i16 z_delta), void (*on_window_focused)(b8 focused),
    void (*on_enter_leave)(b8 e_l),
    void (*on_window_resize)(u16 width, u16 height));

b8 is_fullscreen(void);

b8 is_maximized(void);

void sy_toggle_fullscreen(HWND win);

void sy_toggle_maximize(HWND win);

void window_move(HWND win, i32 x, i32 y, i32 w, i32 h);

void event_fire(Platform* platform);

void platform_window_get_size(const Platform* platform, u16* width,
                              u16* height);

void screen_get_pos(i32* x, i32* y);

void platform_cursor_set_pos(const Platform* platform, i16 x, i16 y);

void platform_cursor_hide(const Platform* platform);

void platform_cursor_show(const Platform* platform);

void platform_mouse_set_pos(const Platform* platform, i16 pos_x, i16 pos_y);

void platform_cursor_show_centered(const Platform* platform);

void platform_mouse_set_last_pos(const Platform* platform);

void platform_cursor_show_last_pos(const Platform* platform);

void platform_cursor_change(const Platform* platform, u32 cursor_id);

void platform_mouse_get_pos(i16* pos_x, i16* pos_y);

double platform_get_time(void);

void platform_sleep(u64 milli);

void platform_shut_down(Platform* platform);

HANDLE file_get_handle(LPCSTR file_path, DWORD operation, DWORD share_mode,
                       DWORD creation);

u32 file_get_size(HANDLE file);

void file_read_bytes(File_Attrib* file_attrib, HANDLE file);

void file_read(File_Attrib* file_attrib, Region_Alloc* region,
               const char* file_path);

void file_write(const char* file_path, const char* content);

void file_write_entire(const char* file_path, const char* content, u32 size);

u32 executable_directory(char* file, u32 size);

