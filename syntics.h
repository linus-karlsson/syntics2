///////// | Syntics\src\buffers.c | //////////////////////

i32 get_type_index(VkPhysicalDeviceMemoryProperties mem_props,
                   VkMemoryRequirements mem_req,
                   VkMemoryPropertyFlags wanted_mem_props);

void map_copy_mem(VkDevice device, Buffer* buffer, void* data);

void map_copy_mem_index(VkDevice device, Index_Buffer* ib);

void map_copy_mem_vertex(VkDevice device, Vertex_Buffer* vb);

void map_copy_unmap_mem(VkDevice device, Buffer* buffer, void* data);

void allocate_memory(VkDevice device, VkPhysicalDevice physical_device,
                     VkMemoryRequirements mem_req,
                     VkMemoryPropertyFlags wanted_mem_props, VkDeviceMemory* memory);

void allocate_commandbuffers(VkDevice device, VkCommandPool command_pool,
                             VkCommandBufferLevel level, u32 command_buffer_count,
                             VkCommandBuffer* command_buffer);

void destroy_buffer(VkDevice device, Buffer buffer);

void destroy_texture(VkDevice device, Texture texture);

void destroy_image(VkDevice device, Image image);

void update_buffers(VkDevice device, Buffer* buffer, void* data, size_t size_bytes);

VkCommandBuffer begin_command_buffer(VkDevice device, VkCommandPool command_pool,
                                     VkCommandBufferLevel level);

void end_command_buffer(VkDevice device, VkCommandPool command_pool,
                        VkCommandBuffer command_buff, VkQueue graphics_queue);

void copy_buffer(VkDevice device, VkCommandPool command_pool, VkBuffer src_buffer,
                 VkBuffer dst_buffer, VkQueue graphics_queue,
                 VkDeviceSize size_bytes);

void create_alloc_bind(VkDevice device, VkPhysicalDevice physical_device,
                       VkMemoryPropertyFlags wanted_mem_props,
                       VkBufferUsageFlags usage_flags, VkBuffer* buffer,
                       VkDeviceMemory* buffer_memory, VkDeviceSize data_size);

void helper_buffer(VkDevice device, VkPhysicalDevice physical_device, void* data,
                   VkBufferUsageFlags usage_flags, Buffer* buffer);

void staging_buffers(VkDevice device, VkPhysicalDevice physical_device,
                     VkCommandPool command_pool, VkQueue graphics_queue,
                     VkBufferUsageFlags vertex_or_index, void* data,
                     VkBuffer* buffer, VkDeviceMemory* buffer_memory,
                     VkDeviceSize size_bytes);

void create_vertex_buffer_test(VkDevice device, VkPhysicalDevice physical_device,
                               Vertex_Buffer* vertex_buffer);

void create_vertex_buffer_visible(VkDevice device, VkPhysicalDevice physical_device,
                                  Vertex_Buffer* vertex_buffer);

void create_vertex_buffer_local(VkDevice device, VkPhysicalDevice physical_device,
                                VkCommandPool command_pool, VkQueue graphics_queue,
                                Vertex_Buffer* vertex_buffer);

void create_index_buffer_test(VkDevice device, VkPhysicalDevice physical_device,
                              Index_Buffer* index_buffer);

void create_index_buffer_visible(VkDevice device, VkPhysicalDevice physical_device,
                                 Index_Buffer* index_buffer);

void create_index_buffer_local(VkDevice device, VkPhysicalDevice physical_device,
                               VkCommandPool command_pool, VkQueue graphics_queue,
                               Index_Buffer* index_buffer);

void create_vertex_index_buffer_default(
    VkDevice device, VkPhysicalDevice physical_device, VkCommandPool command_pool,
    VkQueue graphics_queue, Visible_Local visible_local,
    Vertex_Buffer* vertex_buffer, Index_Buffer* index_buffer);

void create_vertex_index_buffer_default1(VkDevice device,
                                         VkPhysicalDevice physical_device,
                                         VkCommandPool command_pool,
                                         VkQueue graphics_queue,
                                         Visible_Local visible_local,
                                         Vertex_Index_Buffer* vertex_index_buffer);

void create_uniform_buffer(VkDevice device, VkPhysicalDevice physical_device,
                           Uniform_Buffer* uniform_buffer);

void create_uniform_buffer_test(VkDevice device, VkPhysicalDevice physical_device,
                                Uniform_Buffer* uniform_buffer, void** data);

void create_command_pool(VkDevice device, u32 queue_fam_index,
                         VkCommandPool* command_pool);

void update_descritors(Region_Alloc* region, VkDevice device,
                       Descriptors* desciptors, u32 desc_count,
                       const Texture* textures, u32 num_textures,
                       Uniform_Buffer* uniform_buffers);

void create_descriptors(Region_Alloc* region, VkDevice device,
                        Descriptors* desciptors, u32 desc_count,
                        VkDescriptorSetLayout desc_layout, const Texture* texture,
                        u32 num_textures, Uniform_Buffer* uniform_buffers);

void create_image(u32 width, u32 height, VkDevice device,
                  VkPhysicalDevice physical_device, VkFormat format,
                  VkImageTiling tiling, VkImageUsageFlags usage,
                  VkMemoryPropertyFlags wanted_mem_props,
                  VkSampleCountFlagBits num_samples, u32 mip_map_lvl, VkImage* image,
                  VkDeviceMemory* image_mem);

void create_image_view(VkDevice device, VkImage image,
                       VkImageViewType image_view_type, VkFormat image_format,
                       VkImageAspectFlags aspect_mask, u32 mip_map_lvl,
                       VkImageView* image_view);

void create_sampler(VkDevice device, Texture* textue);

void copy_buffer_image(VkDevice device, VkCommandPool command_pool, u32 width,
                       u32 height, u32 mip_map_lvl, VkBuffer src_buffer,
                       VkImage dst_image, VkQueue graphics_queue,
                       VkDeviceSize size_bytes);

void enable_bitmap(VkDevice device, VkCommandPool command_pool,
                   VkQueue graphics_queue, VkImage image, const Texture* texture);

void create_frame_buffer(VkDevice device, VkRenderPass render_pass,
                         VkExtent2D extent_2D, VkImageView img_view,
                         VkImageView depth_view, VkImageView color_view,
                         VkFramebuffer* framebuffer);

u32 rand_rgb(u32 upper, u32 under);

void set_texture_data(VkDevice device, VkPhysicalDevice physical_device, void* data,
                      VkCommandPool command_pool, VkQueue graphics_queue,
                      Texture* texture, VkDeviceSize size_bytes);

i32 max_i(i32 f, i32 s);

void create_texture_path(VkDevice device, VkPhysicalDevice physical_device,
                         VkCommandPool command_pool, VkQueue graphics_queue,
                         b8 mip_map, VkFormat image_format, const char* tex_path,
                         Texture* texture);

u32 create_textures_path(VkDevice device, VkPhysicalDevice physical_device,
                         VkCommandPool command_pool, VkQueue graphics_queue,
                         b8 mip_map, u32 num_textures, const char** tex_paths,
                         Texture* textures);

void create_texture_buffer(VkDevice device, VkPhysicalDevice physical_device,
                           VkCommandPool command_pool, VkQueue graphics_queue,
                           VkFormat image_format, Texture* texture,
                           unsigned char* tex_buffer);

void create_texture(VkDevice device, VkPhysicalDevice physical_device, u32 width,
                    u32 height, VkCommandPool command_pool, VkQueue graphics_queue,
                    Texture* texture);

void create_depth_image(VkDevice device, VkPhysicalDevice physical_device,
                        const VkExtent2D* extent_2D,
                        VkSampleCountFlagBits sample_count, Image* depth_image);

#define sy_RGB(v) ((v) / 255.0f)
void begin_render_pass(VkCommandBuffer command_buffer, VkRenderPass render_pass,
                       VkFramebuffer framebuffer, const VkExtent2D* extent_2D);

void end_render_pass(VkCommandBuffer command_buffer);

void bind_graphics_pipline(VkCommandBuffer command_buffer,
                           const Graphic_Pipeline* graphic_pipline,
                           u32 semaphore_idx);

void push_model(VkCommandBuffer command_buffer, VkPipelineLayout layout, M4 model);

void draw(VkCommandBuffer command_buffer, u32 offset, u32 count);

void bind_vertex_index_buffer(VkCommandBuffer command_buffer,
                              const Vertex_Buffer* vert_buffer,
                              const Index_Buffer* index_buffer);

void bind_vertex_index_buffer1(VkCommandBuffer command_buffer,
                               const Vertex_Index_Buffer* buffer);

void bind_and_draw_graphics_pipline(
    VkCommandBuffer command_buffer, VkDescriptorSet desc_set, u32 index_offset,
    u32 index_count, const Vertex_Buffer* vertex_buffer,
    const Index_Buffer* index_buffer, const Graphic_Pipeline* graphic_pipline,
    const VkViewport* view_port, const VkRect2D* scissor);

void copy_data_buffer(Buffer* buffer, void* data, size_t size_bytes);

///////// | Syntics\src\camera.c | //////////////////////

Camera_3D cam_3dd(void);

Camera_3D cam_3di(f32 speed, f32 sensitivity);

Camera_2D cam_2dd(void);

Camera_2D cam_2di(f32 speed, f32 sensitivity);

b8 update_camera(Camera_3D* camera, const Events* mouse_evt, f32 delta_time,
                 b8 off_the_ground, b8 edit_mode);

void print_camera(const Camera_3D* camera);

///////// | Syntics\src\collision.c | //////////////////////

b8 point_in_point(V2 point_pos, V2 target, V2 target_size);

b8 point_in_rect(V2 point_pos, const AABB_2D* target);

b8 point_in_rect_aabb(V3 point_pos, AABB_3D target);

b8 point_in_entity_2d(V2 point_pos, const Dynamic_Entity_2D* target);

b8 rect_in_rect_normal(const Rect2D* test_obj, const Rect2D* target_obj, V2* normal);

b8 rect_in_rect_2d(const Rect2D* test_obj, const Rect2D* target_obj);

b8 rect_in_rect_3d(const Rect3D* test_obj, const Rect3D* target_obj);

static void swap_f32(f32* first, f32* second);

static b8 ray_rect(V2 ray_origin, V2 ray_direction, const Rect2D* target,
                   V2* contact_point, V2* contact_normal, f32* target_hit_near);

b8 dynamic_ray_rect_unsafe(const Rect2D* test_obj, const Rect2D* target_obj,
                           V2* contact_point, V2* contact_normal, f32* contact_time,
                           f32 dt, f32 low, f32 high);

b8 dynamic_ray_rect_unsafe_d(const Rect2D* test_obj, const Rect2D* target_obj,
                             V2* contact_normal, f32 dt, f32 low, f32 high);

b8 dynamic_ray_rect(const Rect2D* test_obj, const Rect2D* target_obj,
                    V2* contact_point, V2* contact_normal, f32* contact_time,
                    f32 dt);

b8 ray_rect_rects(Rect2D* test_obj, const Rect2D* targets, u32 num_rects, f32 dt);

b8 point_SAT(V2 test, Polygon2D* target);

b8 polygon2D_SAT(Polygon2D* test, Polygon2D* target);

b8 polygon2D_SAT_static(Polygon2D* test, Polygon2D* target, V2* displacement_pos,
                        V2* normal);

b8 polygon2D_lines(Polygon2D* test, Polygon2D* target);

b8 polygon2D_lines_static(Polygon2D* test, Polygon2D* target, V2* displacement_pos);

///////// | Syntics\src\entity.c | //////////////////////

#define MAX_ENTITIES 1000
internal Dynamic_Entity_2D construct_entity(Entity_Movement* move,
                                            Entity_Misc* misc);

void init_entity(Region_Alloc* region);

void update_dyn_etities(void);

Lookup_Key add_dyn_entity(void);

void remove_dyn_entity(Lookup_Key e);

Dynamic_Entity_2D iterate_entities(u32* i);

Entity_Movement* iterate_entity_movement(u32* i);

Entity_Movement* access_dyn_entity_movement(Lookup_Key e);

Dynamic_Entity_2D access_dyn_entity(Lookup_Key e);

///////// | Syntics\src\event_system.c | //////////////////////

#define HIGHEST_KEY_VALUE 191
static void on_key_pressed(u16 key, u16 op);

static void on_key_released(u16 key);

void set_button_unpressed(void);

static void on_button_pressed(u8 button);

static void on_button_released(u8 button);

static void on_mouse_move(i16 pos_x, i16 pos_y);

static void on_mouse_wheel(i16 z_delta);

static void on_window_focused(b8 focused);

static void on_enter_leave(b8 e_l);

static void on_window_resize(u16 width, u16 height);

void init_events(Region_Alloc* region, u32 size);

void subscribe(Events** evt, Event_Type evt_type);

void unsubscribe(Events** evt);

void poll_events(void);

b8 is_key_pressed(u32 key_pressed);

b8 is_any_key_pressed(void);

static b8 check_clicked(b8 pressed, b8* first_clicked);

b8 is_key_clicked(b8* first_clicked, u32 key_pressed);

b8 is_any_key_clicked(b8* first_clicked);

b8 is_any_button_pressed(void);

b8 is_any_button_clicked(b8* first_clicked);

b8 is_window_focused(void);

b8 is_caps_on(void);

u16 code_to_ascii(u16 key);

///////// | Syntics\src\file_reading.c | //////////////////////

b8 end_of_file(const File_Attrib* file);

u32 read_line(File_Attrib* file, char* line, u32 max_size, b8 remove_newline);

char* read_token(char* buffer, u32 buffer_len, const char* delims, u32* token_len);

u32 trim_string(char* string, u32 len);

///////// | Syntics\src\font.c | //////////////////////

#define RESET(thing, bytes) memset(thing, 0, bytes)
#define MAX_WORD_LEN 30
#define READ_HEADER 0
#define READ_CHARS 1
#define READ_CHAR 2
#define READ_ID 0
#define READ_X 1
#define READ_Y 2
#define READ_WIDTH 3
#define READ_HEIGHT 4
#define READ_X_OFFSET 5
#define READ_Y_OFFSET 6
#define READ_X_ADVANCE 7
i32* set_char(Character* chars, i32 i);

#define get_word(file, index, buffer, new_line)                                     \
    do                                                                              \
    {                                                                               \
        if (!_get_word(file, index, buffer, new_line))                              \
        {                                                                           \
            end_of_file = true;                                                     \
            break;                                                                  \
        }                                                                           \
    } while (0)

static b8 _get_word(File_Attrib* file, u32* index, char* buffer, b8* new_line);

Font load_font_file(Region_Alloc* region, const char* file_path);

V2 altas_coords_to_texidx(f32 x, f32 y, f32 atlas_width, f32 atlas_height);

u32 text_3D(Font font, const char* text, V3 pos_first_letter, f32 size,
            f32 win_width, f32 win_height, Vertex** vertices);

f32 text_x_advance(Font font, const char* text, u32 text_len, f32 size);

u32 text_2D_ttf(Font font, const char* text, V3 pos_first_letter, f32 size,
                Vertex** vertices);

u32 text_2D(Font font, f32 y_origin, const char* text, u32 text_len,
            V3 pos_first_letter, V4 color, f32 size, u32* new_lines, float* x_adv,
            Vertex* vertices);

///////// | Syntics\src\game.c | //////////////////////

#define LINES
#define MAX_PARTICLES 4800
#define pack(d, v0, v1, v2)                                                         \
    do                                                                              \
    {                                                                               \
        ASSERT(v0 < 2 && v1 < 0x1FFFFFFF && v2 < 4, "pack to big values");          \
        (d) = ((u32)(v0) << 31) | ((u32)(v1) << 2) | ((u32)(v2)&0x3);               \
    } while (0)

#define unpack_side(d) ((d) >> 31)
#define unpack_curve(d) (((d) >> 2) & 0x1FFFFFFF)
#define unpack_point(d) ((d)&0x3)
AABB_3D aabb_create();

AABB_Representation aabb_rep_create(AABB_3D aabb);

#define DEFAULT_TEXTURE 0
#define OBJ_TEXTURE 1
void aabb_check_min_max(AABB_3D* aabb, V3 pos, V3* current_max);

#define CHUNK_SIZE_X 200
#define CHUNK_SIZE_Y 1
#define CHUNK_SIZE_Z 200
#define CHUNK_SIZE CHUNK_SIZE_X* CHUNK_SIZE_Y* CHUNK_SIZE_Z
#define MAX_THREADS 1
#define multithreaded
f32 round_down_to_half(f32 value);

#define fence _mm_mfence()
#define write_barrier                                                               \
    _WriteBarrier();                                                                \
    fence

#define read_barrier _ReadBarrier()
#define InterlockedIncrement _InterlockedIncrement
void generate_terrain(f32 x_off, f32 z_off, u32 z_chunk_offset, u32 z_chunks,
                      Vertex* verts);

#define chunks CHUNK_SIZE_Z / MAX_THREADS
unsigned long generate_terrain_threaded(void* data);

void generate_normal();

void save_game_binary0(const Bezier_Spline_3D* spline, V3 camera_pos);

void save_game_binary1(Vertex* vert_data, u32* index_data,
                       const Bezier_Spline_3D* spline, V3 camera_pos);

void game_render(void* data, VkCommandBuffer command_buffer, u32 semaphore_idx);

void game_recreate(void* data, Region_Alloc* region,
                   const Application_State* app_state);

void game_destroy(void* data, VkDevice device, u32 num_semaphores);

Bezier_Spline spline_create(Region_Alloc* region, u32 n_curves);

#define PROCENT_INCREASE 0.01f
u32 create_circle(Vertex* data, u32 offset, V3 pos, f32 radius);

u32 create_circles_spline_curve(Vertex* data, u32 offset, Bezier_Spline_3D* spline,
                                u32 curve, f32 radius);

u32 create_circles_spline_2d(Vertex* data, u32 offset, Bezier_Spline* spline,
                             f32 radius);

u32 create_circles_spline_3d(Vertex* data, u32 offset, Bezier_Spline_3D* spline,
                             f32 radius);

V3 brezier_curve_pos(Cubic_Bezier_Curve brezier_curve, f32 t);

u32 generate_curve(Cubic_Bezier_Curve brezier_curve, Vertex* data, u32 offset);

V3 generate_positions_curve(Bezier_Spline_3D* spline, V3 pos, u32 side, u32 curve);

void generate_positions(Bezier_Spline_3D* spline, V3 pos);

u32 generate_spline(Bezier_Spline_3D* spline, Vertex* data, u32 offset);

u32 generate_curve_normals(Vertex* data, u32 offset, u32 spline_splitt,
                           i32 half_splitt, u32 first_index, u32 second_index);

void generate_spline_normals_2d(Vertex* data, u32 offset,
                                const Bezier_Spline* spline);

void generate_spline_normals_3d(Vertex* data, u32 offset,
                                const Bezier_Spline_3D* spline);

void generate_spline_curve(Bezier_Spline_3D* spline, u32 side, u32 curve);

void generate_spline_at_curve(Bezier_Spline_3D* spline, u32 side, u32 curve,
                              u32 point, V3 pos);

void generate_spline_at_curve1(Bezier_Spline_3D* spline, u32 curve);

void generate_positions1(Bezier_Spline* spline, V3 pos);

u32 generate_spline1(Bezier_Spline* spline, Vertex* data, u32 offset);

void generate_spline_at_curve2(Bezier_Spline* spline, u32 curve, u32 point, V3 pos);

f32 get_procent(Bezier_Spline sp, f32 t);

void generate_indices_terrain(u32* index_buffer);

void game_init(Region_Alloc* region, VkDevice device,
               VkPhysicalDevice physical_device, VkCommandPool command_pool,
               VkQueue graphic_queue, const Swap_Chain_Attrib* swap_chain,
               u32 num_semaphores);

void update_gui(Region_Alloc* region, const Application_State* app_state, f32 dt,
                V2 dimensions);

V3 convert_to_noise_coords(V2 x_z);

#define rec_sample_count 1000
b8 record(f32 dt);

V3 mouse_to_device_coords(V3 mouse, V2 dimensions);

V3 shoot_camera_ray(V3 mouse_device_coords);

void swap(f32* x, f32* y);

b8 ray_hit_target_aabb(V3 ray_direction, V3 ray_origin, f32 t, AABB_3D target);

V3 ray_hit(V3 ray, V3 camera_pos, V3 target_pos);

void bubble_sort_rects(Rect3D* rects, u32 size);

void edit_spline(V2 dimensions, b8 camera_moved, V3 ray, b8 first, b8 should_update,
                 b8* hit, b8* xyz_pressed);

f32 point_procent_along_curve_linear(Cubic_Bezier_Curve curve, V3 offset_position,
                                     V3 point_pos, f32 precision);

f32 point_procent_along_curve_binary(Cubic_Bezier_Curve curve, V3 offset_position,
                                     V3 point_pos, f32 precision);

b8 colide_with_spline(const Bezier_Spline_3D* spline, V3 offset_pos, V3 test_pos,
                      V3* collision_pos, V3* normal, b8* side_collision);

void game_update(Region_Alloc* region, const Application_State* app_state,
                 V2 dimensions, u32 semaphore_idx, f32 dt);

///////// | Syntics\src\gui.c | //////////////////////

#define MAX_SPACE 10000
#define BUTTON_SIZE_MULTI 8.3f
#define Y_START_SHADOW gui_ctx.ui_wins[gui_ctx.win_idx].Y_START + 2.0f
#define AABB_START 2
#define AABB_INDEX array_size(win->aabbs) + AABB_START
#define X_START 11.0f
#define Y_START 25.0f
#define AABBS_COUNT_GUI 100
#define LEFT_SIDE_HIT 0
#define RIGHT_SIDE_HIT 1
#define BOTTOM_HIT 2
#define TOTAL_HIT 3
#define LEFT_UPPER_HIT 3
#define LEFT_LOWER_HIT 4
#define RIGHT_UPPER_HIT 5
#define RIGHT_LOWER_HIT 6
#define RESIZE_LEFT 1
#define RESIZE_RIGHT 2
#define RESIZE_TOP 3
#define RESIZE_BUTTOM 4
#define RESIZE_BOTH_RIGHT 5
#define BORDER_THICKNESS 1.0f
#define PADDING 9.0f
#define IDX_OFFSET (gui_ctx.quads_count * INDICES_PER_RECT)
#define INDICES_PER_RECT 6
#define VERTEX_PER_RECT 4
#define TOTAL_NUM_WINS 3
#define DEFAULT_TEXURE 0
#define TEXT_TEXURE 1
#define HEADER_HEIGHT 30
#define RECTS_PER_WINDOW 2000
#define INDICES_PER_WINDOW RECTS_PER_WINDOW * 6
#define VERTICES_PER_WINDOW RECTS_PER_WINDOW * 4
#define TERM_BUFFER_SIZE RECTS_PER_WINDOW - 10
#define GRAPH_BUFFER_SIZE 1000
#define DEFAULT_TEXTURE_GUI 0
#define FONT_TEXTURE_GUI 1
Ui_Window ui_win(u32 id);

Gui gui(void);

Hover_Clicked get_hover_clicked(u32 window_index, u32 aabb_index);

u32 parse_file_binary(void);

void save_file_binary(void);

void gui_init(Region_Alloc* region, VkDevice device,
              VkPhysicalDevice physical_device, VkCommandPool command_pool,
              VkQueue graphic_queue, const Swap_Chain_Attrib* swap_chain,
              u32 num_semaphores, b32 use_save);

void init_terminal(Region_Alloc* region);

void gui_draw(VkCommandBuffer command_buffer, const VkViewport* view_port,
              const VkRect2D* scissor, u32 index_offset, u32 num_indices);

void gui_render(void* data, VkCommandBuffer command_buffer, u32 semaphore_idx);

void recreate(Region_Alloc* region);

void begin_update(Region_Alloc* region, V2 dimensions, u32 semaphore_idx, f32 delta,
                  f32 translucentcy);

static void set_dock_blue(u32 side_hit, V2 pos, V2 size, V2 docked_pos,
                          V2 docked_size);

void end_update(void);

static void change_size(f32* win_dim_to_change, f32* pos_to_change,
                        f32* presist_offset, f32 win_dim, f32 mouse_pos);

static void set_resice(Ui_Window* win, f32* presist_offset, f32 mouse_pos,
                       u32 resize_id);

Window_Handle create_window(void);

void free_window(Window_Handle handle);

void begin_pane(Window_Handle handle, const char* title, V2 pos);

static void move_to_next_chunk(u32* num_indices);

void end_pane(void);

void begin_gridd(u32 x, u32 y);

void end_gridd(void);

static void set_biggest_wide(Ui_Window* win);

static void update_misc(void);

static f32 calculate_text_advance(const char* buffer, u32 len);

static V4 hand_hover(V4 color, b32 hover);

b8 add_button(const char* text);

static b8 is_character_number(u16 key);

static b8 is_character_letter(u16 key);

#define input_focused(curr_input, clicked, allow_letters, cache_on_leave)           \
    _input_focused(&(curr_input)->input, (curr_input)->text,                        \
                   (curr_input)->last_text, sy_SIZE((curr_input)->text), clicked,   \
                   allow_letters, cache_on_leave)

static b8 _input_focused(Input* curr_input, char* text, char* last_text,
                         u32 text_size, b8 clicked, b8 allow_letters,
                         b8 cache_on_leave);

#define render_input(curr_input, win, input_color, text_color, min)                 \
    _render_input(&(curr_input)->input, (curr_input)->text, win, input_color,       \
                  text_color, min)

u32 _render_input(Input* curr_input, const char* text, Ui_Window* win,
                  V4 input_color, V4 text_color, f32 min);

#define add_input_float_d(input, min, max)                                          \
    add_input_float(input, min, max, (max - min) * 0.4f)

b8 add_input_float(f32* input, f32 min, f32 max, f32 speed);

b8 add_input_text(char* ptr_to_text, u32* size);

void add_text(const char* text);

static u32 flush_buffer(void** s_buffer, u32 size_bytes, f32 multiplier);

static u32 flush_graph(void);

static void flush_terminal(void);

void add_terminal(f32 width, f32 height);

void add_graph(f32 value, const char* y_title, f32 y_max, f32 y_min, f32 sample_rate,
               f32 dt);

static b8 showcase_entity(Dynamic_Entity_2D* e, Ui_Window* win, char* name);

void edit_show_entity(Dynamic_Entity_2D* e, char* name);

void show_entity(Dynamic_Entity_2D* e, char* name);

void entity_watch_window(void);

void gui_destroy(VkDevice device, u32 num_semaphores);

b8 is_focus(void);

void sy_print_text(char* text);

///////// | Syntics\src\instance_device.c | //////////////////////

VkInstance get_instance(void);

VkDebugUtilsMessengerEXT get_debug_messenger(void);

VKAPI_ATTR VkBool32 VKAPI_CALL msg_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

VkDebugUtilsMessengerCreateInfoEXT config_debug_info(void);

void init_debug_messenger(void);

void destroy_debug_messenger(VkInstance instance,
                             VkDebugUtilsMessengerEXT debugMessenger,
                             const VkAllocationCallbacks* pAllocator);

void init_instance(Region_Alloc* region);

Queue_Family_Indices get_queue_indices(Region_Alloc* region,
                                       VkPhysicalDevice physical_device,
                                       VkSurfaceKHR surface, b8* all_supported);

void pick_physical_device(Region_Alloc* region, VkInstance instance,
                          VkSurfaceKHR surface, VkPhysicalDevice* physical_device,
                          Queue_Family_Indices* q_indices);

void create_logical_device(VkPhysicalDevice physical_device,
                           Queue_Family_Indices q_indices, VkDevice* device);

void destroy_instance(void);

///////// | Syntics\src\linux\linux_platform.c | //////////////////////

const Linux_Platform& get_platform_state();

void init_platform(const char* title, u16 width, u16 height);

xcb_window_t child_window(const char* title, u16 width, u16 height);

void set_event_callbacks(void (*on_key_pressed)(u16 key, u16 op),
                         void (*on_key_released)(u16 key, u16 op),
                         void (*on_button_pressed)(u8 key, u16 op),
                         void (*on_button_released)(u8 key, u16 op),
                         void (*on_mouse_move)(i16 pos_x, i16 pos_y, u16 op),
                         void (*on_window_focused)(b8 focused, u16 op),
                         void (*on_enter_leave)(b8 e_l, u16 op));

void change_title(const char* title, u32 len);

void event_fire();

void move_main_window();

void get_window_size(u16& width, u16& height);

void hide_cursor();

void show_cursor();

void show_cursor_centered();

void show_cursor_last_pos();

void change_cursor(u32 cursor_id);

void set_mouse_pos(i16 pos_x, i16 pos_y);

void set_mouse_last_pos();

void get_pos(i16& pos_x, i16& pos_y);

double get_time();

void platform_sleep(u32milli);

void shut_down_platform();

void read_file(File_Attrib* file_attrib, Region_Alloc* region, const char* file_path,
               const char* operation);

///////// | Syntics\src\logging.c | //////////////////////

void init_logging(void);

void set_log(b8 set_val);

b8 use_log(void);

void set_log_alloc(b8 set_val);

b8 use_log_alloc(void);

char* line_file_to_buffer(const char* file, i32 line, const char* msg);

void _ERROR(const char* file, i32 line, const char* msg);

void sy_print(const char* format, ...);

///////// | Syntics\src\lookup_table.c | //////////////////////

Lookup_Table lookup_table_create(Region_Alloc* region, u32 n_entries);

Lookup_Key add_entry(Lookup_Table* table, u32 ref_index);

u32 table_index(Lookup_Table* table, Lookup_Key key);

u32 remove_entry(Lookup_Table* table, Lookup_Key key);

void cange_entry_index(Lookup_Table* table, u32 entry, u32 new_index);

///////// | Syntics\src\main.c | //////////////////////

///////// | Syntics\src\math\syntics_math.c | //////////////////////

V2 v2d(void);

V2 v2i(f32 i);

V2 v2f(f32 x, f32 y);

V2 v2_v3(V3 v3);

V2 v2_v4(V4 v4);

V3 v3d(void);

V3 v3i(f32 i);

V3 v3f(f32 x, f32 y, f32 z);

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

f32 v3_len_squared(V3 v3);

f32 v3_len(V3 v3);

V3 v3_lerp(V3 v1, V3 v2, f32 t);

f32 v2_dot(V2 v1, V2 v2);

f32 v3_dot(V3 v1, V3 v2);

f32 v3_angle(V3 v1, V3 v2);

V2 v2_normalize(V2 v2);

V3 v3_normalize(V3 v3);

V3 v3_normalize_len(V3 v3, f32 len);

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

P3 p3_floor(P3 p);

P3 p3_ceil(P3 p);

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

M4 m4_rotate(f32 rad, Axis axis);

V3 v3_rotate(V3 v3, f32 rad, V3 normal);

M3 translate(V2 v);

M4 m4_translate(V3 v3);

M3 scale(V2 v);

M4 m4_scale(V3 v);

M4 ortho(f32 left, f32 right, f32 bottom, f32 top, f32 sy_near, f32 sy_far);

M4 view(V3 eye, V3 center, V3 up);

M4 perspective(f32 fov, f32 aspect, f32 sy_near, f32 sy_far);

M4 inverse(M4 m);

b8 is_poly2d_convex(Polygon2D p);

Plane plane(P3 a, P3 b, P3 c);

///////// | Syntics\src\noise.c | //////////////////////

f32 sy_normalize_f32(f32 value, f32 min, f32 max);

f32 sy_fade(f32 t);

f32 sy_lerp(f32 a, f32 b, f32 t);

i32 sy_noise2(i32 x, i32 y);

f32 sy_smooth_inter(f32 a, f32 b, f32 t);

f32 sy_noise2d(f32 x, f32 y);

f32 sy_value_noise2d(f32 x, f32 y, f32 freq, f32 gain, i32 oct);

///////// | Syntics\src\random.c | //////////////////////

void set_seed(void);

u32 rand_uint(u32 low, u32 high);

f32 rand_f32(f32 low, f32 high);

///////// | Syntics\src\region_alloc.c | //////////////////////

Array_Head array_head_create(u32 capacity, u32 size);

b8 init_region(Region_Alloc* region, u64 size);

void init_stack(u32 size);

Region_Alloc* _get_stack(u32 check_val);

void reset_stack(void);

u64 _stack_begin_scope(void);

#define MAX(val1, val2) ((val1) > (val2) ? (val1) : (val2))
void _stack_end_scope(u64 size_at_start);

Region_Alloc region_alloc(void);

static void* init_malloc(Region_Alloc* region, u32 size, Alloc_Type alloc_type);

void* _region_malloc(Region_Alloc* region, u32 size, Alloc_Type alloc_type);

void* _region_calloc(Region_Alloc* region, u32 size, Alloc_Type alloc_type);

void _region_pop(Region_Alloc* region, u32 size, Alloc_Type alloc_type);

void reset_region(Region_Alloc* region);

void free_region(Region_Alloc* region);

void print_region(const Region_Alloc* region);

static void* init_array(Region_Alloc* region, u32 capacity, u32 type,
                        Alloc_Type alloc_type, u32 extra_size);

void* _dyn_array(Region_Alloc* region, u32 capacity, u32 type, Alloc_Type alloc_type,
                 u32 extra_size);

void* _dyn_array_calloc(Region_Alloc* region, u32 capacity, u32 type,
                        Alloc_Type alloc_type);

void* _dyn_array_val(Region_Alloc* region, u32 capacity, u32 type,
                     Alloc_Type alloc_type, const void* values);

Array_Head* _check_array(void* array);

b8 _check_array_size(void* array);

u32 _check_array_size_index(void* array, u32 index);

u32 _check_pop_array_size(void* array);

void _array_clear(void* array, u32 stride);

u32 array_size(const void* const array);

u32 array_capacity(const void* const array);

u32 _get_id(void);

#define extend_path_d0(region, path) extend_path(region, path, (u32)strlen(path))
#define extend_path_d1(path) extend_path(get_stack(), path, (u32)strlen(path))
char* extend_path(Region_Alloc* region, const char* trailing_path,
                  u32 trailing_path_len);

///////// | Syntics\src\render.c | //////////////////////

#define TEST_BED
#define RENDER_MAX_SPACE 100
void create_fence_semaphore(VkDevice device, VkFence* fence,
                            VkSemaphore* image_semaphores,
                            VkSemaphore* present_semaphores);

void init_render_state(Region_Alloc* region, VkDevice device, Queues queues,
                       VkPhysicalDevice physical_device, VkCommandPool command_pool,
                       const Queue_Family_Indices* q_indices, u32 num_semaphores,
                       const Swap_Chain_Attrib* swap_chain);

void draw_pipeline(void (*draw_callback)(void* data, VkCommandBuffer command_buffer,
                                         u32 semaphore_idx),
                   void* data);

void subscribe_recreate_callback(
    void (*rc_callback)(void* data, Region_Alloc* region,
                        const Application_State* app_state),
    void* data);

void subscribe_destroy_callback(void (*destroy_callback)(void* data, VkDevice device,
                                                         u32 num_semaphores),
                                void* data);

void submit_and_present(VkQueue graphic_queue, VkQueue present_queue,
                        VkSemaphore image_semaphore, VkSemaphore present_semaphore,
                        VkFence fence, VkCommandBuffer* command_buffers,
                        u32 command_buffer_count, VkSwapchainKHR swap_chain,
                        u32 image_index);

void render(Region_Alloc* region, Application_State* app_state, f32 dt);

void destroy_render_state(void);

///////// | Syntics\src\render_util.c | //////////////////////

AABB_2D _set_up_verticies(Vertex* vertices, u32* rect_count, V3 pos, V2 size,
                          V4 color, f32 tex_index, Tex_Coords tex_coords);

AABB_2D quad(Vertex* vertices, u32* rect_count, V3 pos, V2 size, V4 color,
             f32 tex_index);

AABB_2D quad_f(Vertex* vertices, u32* rect_count, V3 pos, V2 size, V4 color,
               f32 tex_index);

AABB_2D quad_gradiant_l_r(Vertex* vertices, u32* rect_count, V3 pos, V2 size,
                          V4 left_color, V4 right_color, f32 tex_index);

AABB_2D quad_gradiant_t_b(Vertex* vertices, u32* rect_count, V3 pos, V2 size,
                          V4 top_color, V4 bottom_color, f32 tex_index);

AABB_2D quad_s_gradiant_l_r(Vertex* vertices, u32* rect_count, V3 pos, V2 size,
                            V4 left_color, V4 right_color, f32 tex_index,
                            f32 shadow_offset);

AABB_2D quad_s_gradiant_t_b(Vertex* vertices, u32* rect_count, V3 pos, V2 size,
                            V4 top_color, V4 bottom_color, f32 tex_index,
                            f32 shadow_offset);

AABB_2D quad_s_gradiant(Vertex* vertices, u32* rect_count, V3 pos, V2 size, V4 color,
                        f32 multiplier, f32 tex_index, f32 shadow_offset);

AABB_2D quad_s(Vertex* vertices, u32* rect_count, V3 pos, V2 size, V4 color,
               f32 tex_index, f32 shadow_offset);

AABB_2D quad_sl(Vertex* vertices, u32* rect_count, V3 pos, V2 size, V4 color,
                f32 tex_index, f32 shadow_offset);

AABB_2D quad_sl_gradiant(Vertex* vertices, u32* rect_count, V3 pos, V2 size,
                         V4 color, f32 tex_index, f32 shadow_offset);

AABB_2D quad_r(Vertex* vertices, u32* rect_count, V3 pos, V2 size, V4 color,
               f32 tex_index, f32 rotation, V2 dimensions);

AABB_2D add_border_s(Vertex* data, u32* num_indices, V4 border_color, V3 top_left,
                     V2 size, f32 thickness, f32 tex_index);

AABB_2D add_border(Vertex* data, u32* num_indices, V4 border_color, V3 top_left,
                   V2 size, f32 thickness, f32 tex_index);

void quad_middle(Vertex* vertices, V3 pos, V2 size, V4 color, f32 tex_index);

void polygon2D_draw_quads(Vertex* data, Polygon2D poly, f32 z, V4 color,
                          f32 line_width, f32 tex_index);

void polygon2D_draw_lines(Vertex* data, u32* idx_data, Polygon2D poly, f32 z,
                          V4 color, f32 tex_index);

internal void insert_indices(u32* idx_data, u32 p_i, u32 added_val0, u32 added_val1);

void square_rounded_corners(Vertex* vert_data, u32* idx_data, V3 pos, V2 size,
                            V4 color, f32 seperation, u32 corner_vertices_count,
                            f32 tex_index);

void generate_indices(u32* data, u32 offset, u32 indices_count);

u32 cube(Vertex* vertices, u32 offset, V3 pos, V3 size, V4 color, f32 tex_index);

void cube1(Vertex* vertices, V3 pos, V3 size, V4 color, f32 tex_index);

void cube_not_center1(Vertex* vertices, V3 pos, V3 size, V4 color, f32 tex_index);

u32 cube_not_center(Vertex* vertices, u32 offset, V3 pos, V3 size, V4 color,
                    f32 tex_index);

void cube_indices_offset(u32* indices, u32 offset, u32 how_many);

void cube_indices(u32* indices, u32 offset, u32 how_many);

u32 gridd_using_line_list(Vertex* vertices, u32 vertex_offset, u32* indices,
                          u32 index_offset, V3 middle_pos, V2 spacing,
                          u32 lines_width_count, u32 lines_height_count, V4 color,
                          f32 tex_index);

///////// | Syntics\src\simple_particle.c | //////////////////////

void init_particles_2d(Region_Alloc* region, Particles_2D* particles,
                       u32 max_particles);

void emit_particle_2d(Particles_2D* particles,
                      const Particle_Attrib_2D* particle_attrib, V2 individual_speed,
                      V2 neg_alt, f32 life);

u32 update_particles_2d(Particles_2D* particles, Vertex* vertices, f32 dt);

void init_particles_3d(Region_Alloc* region, Particles_3D* particles,
                       u32 max_particles);

void emit_particle_3d(Particles_3D* particles,
                      const Particle_Attrib_3D* particle_attrib, V3 individual_speed,
                      V3 neg_alt, f32 random, f32 life);

u32 update_particles_3d(Particles_3D* particles, Vertex* vertices, u32 vertex_offset,
                        f32 dt);

///////// | Syntics\src\swap_chain.c | //////////////////////

u32 clamp_u32(u32 value, u32 min, u32 max);

VkSampleCountFlagBits max_usable_sample_count(VkPhysicalDevice physical_device);

void create_swapchain(VkPhysicalDevice physical_device, VkDevice device,
                      VkSurfaceKHR surface, u32 width, u32 height,
                      Queue_Family_Indices indices, VkSwapchainKHR old_swap_chain,
                      Swap_Chain_Attrib* swap_chain);

void create_render_pass(VkDevice device, VkFormat color_format,
                        VkSampleCountFlagBits sample_count,
                        VkRenderPass* render_pass);

void get_swapchain_images(Region_Alloc* region, VkDevice device,
                          Swap_Chain_Attrib* swap_chain);

void create_graphics_pipeline(VkDevice device, VkRenderPass render_pass,
                              VkSampleCountFlagBits sample_count,
                              const char* vert_path, const char* frag_path,
                              u32 width, u32 height, u32 num_textures,
                              const VkRect2D* sciss,
                              Graphic_Pipeline* graphic_pipline);

void init_uniforms_descriptors(Region_Alloc* region, VkDevice device,
                               VkPhysicalDevice physical_device,
                               Uniform_Buffer** uniform_buffers,
                               Descriptors* descriptors,
                               VkDescriptorSetLayout set_layout, u32 num_semaphores,
                               const Texture* textures, u32 num_textures);

void init_graphics_pipeline(Region_Alloc* region, VkDevice device,
                            VkPhysicalDevice physical_device, u32 num_semaphores,
                            const Texture* textures, u32 num_textures,
                            Graphic_Pipeline* gp);

void create_graphics_pipeline_deluxe(Region_Alloc* region, VkDevice device,
                                     VkPhysicalDevice phy_device, u32 num_semaphores,
                                     const char* vert_path, const char* frag_path,
                                     const Swap_Chain_Attrib* swap_chain,
                                     const Texture* textures, u32 num_textures,
                                     Graphic_Pipeline* graphic_pipline);

void enable_multisample(const Swap_Chain_Attrib* swap_chain, VkDevice device,
                        VkPhysicalDevice physical_device, Image* color_image);

void recreate_graphic_pipline_ap(const Application_State* app_state,
                                 const char* vert_file, const char* frag_file,
                                 Graphic_Pipeline* graphic_pipline, u32 num_textures,
                                 const VkRect2D* scissor);

void recreate_graphic_pipline_sw(VkDevice device,
                                 const Swap_Chain_Attrib* swap_chain,
                                 const char* vert_file, const char* frag_file,
                                 Graphic_Pipeline* graphic_pipline, u32 num_textures,
                                 const VkRect2D* scissor);

void recreate_swapchain(Region_Alloc* region, Application_State* app_state,
                        u32 width, u32 height);

void destroy_graphic_pipeline(VkDevice device, u32 num_semaphores,
                              Graphic_Pipeline* gp);

///////// | Syntics\src\syntics.c | //////////////////////

///////// | Syntics\src\syntic_app.c | //////////////////////

#define PRINT_REGION
void find_working_dir(Region_Alloc* region);

void run_app(void);

///////// | Syntics\src\test_bed.c | //////////////////////

#define DEFAULT_TEXTURE_TEST 0
#define FONT_TEXTURE_TEST 1
internal void test_bed_destroy(void* data, VkDevice device, u32 num_semaphores);

void test_bed_render(void* data, VkCommandBuffer command_buffer, u32 semaphore_idx);

u32 circle(Vertex* vertices, u32 vertex_offset, u32* indices, u32 index_offset,
           u32* indices_count, V3 middle_pos, u32 triangle_count, f32 radius,
           V4 color, f32 tex_index);

unsigned long looking_for_file_changes(void* data);

void test_bed_init(Region_Alloc* region, VkDevice device,
                   VkPhysicalDevice physical_device, VkCommandPool command_pool,
                   VkQueue graphic_queue, const Swap_Chain_Attrib* swap_chain,
                   u32 num_semaphores);

void test_update_gui(Region_Alloc* region, const Application_State* app_state,
                     f32 dt, V2 dimensions);

void test_bed_recreate_gps(const Application_State* app_state);

void test_bed_update(Region_Alloc* region, const Application_State* app_state,
                     V2 dimensions, u32 semaphore_idx, f32 dt);

///////// | Syntics\src\vulkan_api.c | //////////////////////

void init_vulkan(Region_Alloc* region, Application_State* app_state, u32 width,
                 u32 height);

void destroy_vulkan(void);

///////// | Syntics\src\win32\win32_platform.c | //////////////////////

#define SYNT_NORMAL_CURSOR 0
#define SYNT_HAND_CURSOR 1
#define SYNT_RESIZE_H_CURSOR 2
#define SYNT_RESIZE_V_CURSOR 3
#define SYNT_RESIZE_NW_CURSOR 4
#define SYNT_MOVE_CURSOR 5
#define SYNT_HIDDEN_CURSOR 6
#define TOTAL_CURSORS 7
void* thread_create(void* data, unsigned long (*thread_function)(void* data),
                    unsigned long creation_flag, unsigned long* thread_id);

void close_handle(void* handle);

void error_msg(const char* msg);

HWND get_win(void);

#define SYNT_KEY_CAPS 20
#define SYNT_KEY_SHIFT 16
LRESULT msg_handler(HWND win, UINT msg, WPARAM w_param, LPARAM l_param);

static void sy_fullscreen(HWND window);

void init_platform(const char* title, u16* width, u16* height, b32 full_screen);

void set_event_callbacks(
    void (*on_key_pressed)(u16 key, u16 op), void (*on_key_released)(u16 key),
    void (*on_button_pressed)(u8 key), void (*on_button_released)(u8 key),
    void (*on_mouse_move)(i16 pos_x, i16 pos_y), void (*on_mouse_wheel)(i16 z_delta),
    void (*on_window_focused)(b8 focused), void (*on_enter_leave)(b8 e_l),
    void (*on_window_resize)(u16 width, u16 height));

b8 is_fullscreen(void);

b8 is_maximized(void);

void sy_toggle_fullscreen(void);

void sy_toggle_maximize(void);

void sy_move_window(i32 x, i32 y, i32 w, i32 h);

void event_fire(void);

void get_window_size(u16* width, u16* height);

void get_screen_pos(i32* x, i32* y);

static void set_cursor_pos(i16 x, i16 y);

void hide_cursor(void);

void show_cursor(void);

void show_cursor_centered(void);

void show_cursor_last_pos(void);

void change_cursor(u32 cursor_id);

void set_mouse_pos(i16 pos_x, i16 pos_y);

void set_mouse_last_pos(void);

void get_pos(i16* pos_x, i16* pos_y);

double get_time(void);

void platform_sleep(u64 milli);

void shut_down_platform(void);

HANDLE get_file_handle(LPCSTR file_path, DWORD operation, DWORD share_mode,
                       DWORD creation);

u32 get_size(HANDLE file);

void read_bytes(File_Attrib* file_attrib, HANDLE file);

void read_file_offset_arr(File_Attrib* file_attrib, Region_Alloc* region,
                          const char* file_path, const char* operation);

void read_file(File_Attrib* file_attrib, Region_Alloc* region, const char* file_path,
               const char* operation);

void write_to_file(const char* file_path, const char* content);

void write_entire_file(const char* file_path, const char* content, u32 size);

