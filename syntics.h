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


///////// | Syntics\src\game.c | //////////////////////

