#include "logging.h"
#include "region_alloc.h"
#include "font.h"
#include "camera.h"
#include "buffers.h"
#include "swap_chain.h"
#include "gui.h"
#include "event_system.h"
#include "random.h"
#include "noise.h"
#include <math.h>
#include <Windows.h>

typedef struct Terrain_State
{
    Graphic_Pipline g_pipline;

    Camera cam;

    Texture* textures;
    Font font;
    Events* mouse_evt;
} Terrain_State;

static Terrain_State terrain_state;

static const f32 QUAD_WIDTH = 0.5f;
static const f32 QUAD_HEIHT = -0.5f;

static const u32 TERRAIN_SIZE_X = 300;
static const u32 TERRAIN_SIZE_Z = 200;

static const u32 TERRAIN_SIZE = TERRAIN_SIZE_X * TERRAIN_SIZE_Z;

static f32 freq = 0.41f;
static f32 grain = 0.36f;
static f32 oct = 3.0f;
static f32 max_heigt = 8.0f;

static void generate_terrain(f32 x_off, f32 z_off)
{
    Vertex_Buffer* vert = &terrain_state.g_pipline.vert_buffer;

    for_range(z, TERRAIN_SIZE_Z)
    {
        f32 ix_off = x_off;
        for_range(x, TERRAIN_SIZE_X)
        {
            f32 random_f =
                (sy_value_noise2d(ix_off, z_off, freq, grain, (int32)oct) *
                 max_heigt);
#if 0
            char temp[15] = {};
            sprintf(temp, "%f\n", random_ff);
            OutputDebugString(temp);
#endif

            // f32random_f = rand_f32(0.0f, 1.0f);
            Vec4 pos = Vec4(x * QUAD_WIDTH, random_f, z * QUAD_HEIHT, 1.0f);
            f32 colorf = random_f / max_heigt;
            Vec4 color = Vec4(colorf, colorf, colorf, 1.0f);
            color.w = 1.0f;
            f32 tex_index = 0.0f;

            Vertex vertex = { pos, color, Vec2(ix_off, z_off), tex_index };

            synt_push(vert->data, vertex);

            ix_off += 0.1f;
        }
        z_off += 0.1f;
    }
}

static f32 calculate_procentage(f32 value, f32 low, f32 high)
{
    return (value - low) / (high - low);
}

static f32 max_slope = 0.01f;
static void update_terrain_in_CPU(f32 x_off, f32 z_off)
{
    generate_terrain(x_off, z_off);

    max_slope = 0.01;
    Vertex_Buffer* vert = &terrain_state.g_pipline.vert_buffer;
    u32 size = size_arr(vert->data);
    Vec3 up = Vec3(0.0f, 1.0f, 0.0f);
    for (int i = 0; i < size - TERRAIN_SIZE_Z - 2; i += 1)
    {
        Vec4 pos = vert->data[i].pos;
        Vec4 next_pos0 = vert->data[i + TERRAIN_SIZE_X].pos;
        Vec4 next_pos1 = vert->data[i + 1].pos;
        Vec3 side0 = next_pos0 - pos;
        Vec3 side1 = next_pos1 - pos;
        Vec3 normal = normalize(cross(side1, side0));
        f32 slope = acosf(dot(normal, up));
        if (slope > max_slope && slope < 1.0f)
        {
            max_slope = slope;
        }

        // f32slope = acosf(dot(normal, up));

        vert->data[i].color = Vec4(normal.x, normal.y, normal.z, max_slope);
    }
}

static void update_terrain_in_GPU(f32 x_off, f32 z_off)
{
    Vertex_Buffer* vert = &terrain_state.g_pipline.vert_buffer;

    int32 idx = 0;
    for_range(z, TERRAIN_SIZE_Z)
    {
        f32 ix_off = x_off;
        for_range(x, TERRAIN_SIZE_X)
        {
            vert->data[idx++].tex_coords = Vec2(ix_off, z_off);
            ix_off += 0.1f;
        }
        z_off += 0.1f;
    }
}

static void update_voxel_test(f32 x_off, f32 z_off)
{
    Vertex_Buffer* vert = &terrain_state.g_pipline.vert_buffer;
    Index_Buffer* idx = &terrain_state.g_pipline.idx_buffer;

    u32 z_max = TERRAIN_SIZE_Z / 2;
    u32 x_max = TERRAIN_SIZE_X / 2;

    int32 size = size_arr(idx->data);

    int32 i = 0;
    for_range(z, z_max - 1)
    {
        i += (TERRAIN_SIZE_X * 2);
        f32 ix_off = x_off;
        for_range(x, x_max)
        {
            vert->data[idx->data[i++]].tex_coords = Vec2(ix_off, z_off);
            vert->data[idx->data[i++]].tex_coords = Vec2(ix_off, z_off);
            vert->data[idx->data[i++]].tex_coords = Vec2(ix_off, z_off);
            vert->data[idx->data[i++]].tex_coords = Vec2(ix_off, z_off);
            ix_off += 0.1f;
        }
        z_off += 0.1f;
    }
}

void init_terrain(Region_Alloc* region, VkDevice device,
                  VkPhysicalDevice physical_device, VkCommandPool command_pool,
                  VkQueue graphic_queue, const Swap_Chain_attrib& swap_chain,
                  u32 num_semaphores)
{

    terrain_state.textures = dyn_arrayP(region, 2, Texture);

    create_texture(device, physical_device, command_pool, graphic_queue, true,
                   VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/default.png",
                   &terrain_state.textures[0]);

    get_head(terrain_state.textures)->size++;

    terrain_state.g_pipline.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    create_graphics_pipeline(
        region, device, swap_chain.color_format, swap_chain.render_pass,
        swap_chain.sample_count, "Syntics/res/terrain.vert.spv",
        "Syntics/res/terrain.frag.spv", swap_chain.extent_2D.width,
        swap_chain.extent_2D.height, VK_CULL_MODE_NONE,
        size_arr(terrain_state.textures), NULL, &terrain_state.g_pipline);

    terrain_state.g_pipline.vert_buffer.data =
        dyn_arrayP(region, (TERRAIN_SIZE)*1, Vertex);

    terrain_state.g_pipline.idx_buffer.data =
        dyn_arrayP(region, (TERRAIN_SIZE)*2, uint32);

    update_terrain_in_CPU(0.0f, 0.0f);

    Index_Buffer* idx = &terrain_state.g_pipline.idx_buffer;
    Vertex_Buffer* vert = &terrain_state.g_pipline.vert_buffer;

    int32 I = 0;
    int32 step_value = 1;
    for_range(i, TERRAIN_SIZE_Z - 1)
    {
        for_range(j, TERRAIN_SIZE_X)
        {
            synt_push(idx->data, (TERRAIN_SIZE_X * i) + I);
            synt_push(idx->data, (TERRAIN_SIZE_X * (i + 1)) + I);

            I += step_value;
        }
        step_value *= -1;
        I += step_value;
    }

    terrain_state.g_pipline.vert_buffer.size_bytes =
        capacity_arr(terrain_state.g_pipline.vert_buffer.data) * sizeof(Vertex);
    create_vertex_buffer(device, physical_device, command_pool, graphic_queue,
                         &terrain_state.g_pipline.vert_buffer);

    terrain_state.g_pipline.idx_buffer.size_bytes =
        size_arr(terrain_state.g_pipline.idx_buffer.data) * sizeof(uint32);
    terrain_state.g_pipline.idx_buffer.curr_size =
        size_arr(terrain_state.g_pipline.idx_buffer.data);
    create_index_buffer(device, physical_device, command_pool, graphic_queue,
                        &terrain_state.g_pipline.idx_buffer);

    terrain_state.g_pipline.uniform_buffers =
        region_mallocP(region, num_semaphores, Uniform_Buffer);
    terrain_state.g_pipline.descriptors.desc_sets =
        region_mallocP(region, num_semaphores, VkDescriptorSet);

    for_range(i, num_semaphores)
    {
        terrain_state.g_pipline.uniform_buffers[i].size_bytes = (uint32)sizeof(MVP);

        create_uniform_buffer(device, physical_device,
                              &terrain_state.g_pipline.uniform_buffers[i]);
    }

    create_descriptors(region, device, &terrain_state.g_pipline.descriptors,
                       num_semaphores, terrain_state.g_pipline.set_layout,
                       terrain_state.textures, size_arr(terrain_state.textures),
                       terrain_state.g_pipline.uniform_buffers);

    terrain_state.cam.position = v3f(52.0f, 15.15f, -10.5f);
    terrain_state.cam.orientation = v3f(0.0f, 0.0f, -1.0f);

    terrain_state.cam.speed = 10.0f;

    terrain_state.cam.mvp.model = scale(mat4i(1.0f), v3f(1.0f, 1.0f, 1.0f));
    terrain_state.cam.mvp.view =
        view(terrain_state.cam.position,
             terrain_state.cam.position + terrain_state.cam.orientation,
             terrain_state.cam.up);

    terrain_state.cam.mvp.light_pos = Vec3(1.0f, 5.0f, 0.5f);

    subscribe(&terrain_state.mouse_evt, EVT_MOUSE);
}

static f32 translucentcy = 0.8f;

static bool new_window = false;
static bool new_window2 = false;

static void update_gui(Region_Alloc* region, f32 dt)
{
    back_bord_begin("TTTT", Vec2(100.0f));
    {
        gridd_begin(2, 1);
        {
            add_text("Translucentcy: ");
            add_input_float(translucentcy, 0.0f, 1.0f);
        }
        gridd_end();
        gridd_begin(4, 1);
        {
            if (add_button("OFF"))
            {
                translucentcy = 0.0f;
            }
            if (add_button("Low"))
            {
                translucentcy = 0.2f;
            }
            if (add_button("Hi"))
            {
                translucentcy = 0.8f;
            }
            if (add_button("Fill"))
            {
                translucentcy = 1.0f;
            }
        }
        gridd_end();
        gridd_begin(2, 1);
        {
            if (add_button("New window"))
            {
                new_window = new_window ? false : true;
            }
            if (add_button("New window2"))
            {
                new_window2 = new_window2 ? false : true;
            }
        }
        gridd_end();
        gridd_begin(1, 1);
        {
            add_text("Freq - Grain - Oct - max height");
        }
        gridd_end();
        gridd_begin(4, 1);
        {
            add_input_float(freq, 0.0f, 1.0f);
            add_input_float(grain, 0.0f, 2.0f);
            add_input_float(oct, 0.0f, 10.0f);
            add_input_float(max_heigt, 0.0f, 20.0f);
        }
        gridd_begin(1, 1);
        {
            char* text = NULL;
            u32 size = 0;
            if (add_input_text(&text, &size))
            {
                synt_LOG_Term("Text: %s\nSize: %u\n", text, size);
            }
        }
        gridd_end();

        gridd_begin(1, 1);
        {
            static char temp[60] = {};
            static f32 count = 1.0f;
            if (count >= 0.1f)
            {
                u32 fps = (uint32)(1.0f / dt);
                f32 milli = dt * 1000.0f;
                sprintf(temp, "Milli: %f | FPS: %u", milli, fps);
                count = 0.0f;
            }
            count += dt;
            add_text(temp);
        }
    }
    back_bord_end();
    back_bord_begin("Terminal", Vec2(500.0f, 100.0f));
    {
        add_terminal(250.0f, 200.0f);
    }
    back_bord_end();
    if (new_window)
    {
        back_bord_begin("Eeeeh okkkeeh", Vec2(500.0f));
        {
            gridd_begin(2, 1);
            {
                add_text("Translucentcy: ");
                add_input_float(translucentcy, 0.0f, 1.0f);
            }
            gridd_end();
            gridd_begin(4, 1);
            {
                if (add_button("OFF"))
                {
                    translucentcy = 0.0f;
                }
                if (add_button("Low"))
                {
                    translucentcy = 0.2f;
                }
                if (add_button("High"))
                {
                    translucentcy = 0.8f;
                }
                if (add_button("Fill"))
                {
                    translucentcy = 1.0f;
                }
            }
            gridd_end();
            gridd_begin(1, 1);
            {
                add_text("Freq --- Grain --- Oct ------- max Slope");
            }
            gridd_end();
            gridd_begin(4, 1);
            {
                add_input_float(freq, 0.0f, 1.0f);
                add_input_float(grain, 0.0f, 2.0f);
                add_input_float(max_slope, 0.0f, 1.0f);
                add_input_float(max_slope, 0.0f, 1.0f);
            }
            gridd_end();
            gridd_begin(1, 1);
            {
                char* text = NULL;
                u32 size = 0;
                if (add_input_text(&text, &size))
                {
                    synt_LOG_Term("Text: %s\nSize: %u\n", text, size);
                }
            }
            gridd_end();

            gridd_begin(1, 1);
            {
                static char temp[60] = {};
                static f32 count = 1.0f;
                if (count >= 0.1f)
                {
                    u32 fps = (uint32)(1.0f / dt);
                    f32 milli = dt * 1000.0f;
                    sprintf(temp, "Milli: %f | FPS: %u", milli, fps);
                    count = 0.0f;
                }
                count += dt;
                add_text(temp);
            }
            gridd_end();
        }
        back_bord_end();
    }
    if (new_window2)
    {
        back_bord_begin("Eeeh", Vec2(200.0f));
        {
            gridd_begin(2, 1);
            {
                if (add_button("OFdd"))
                {
                    translucentcy = 0.0f;
                }
                if (add_button("Low"))
                {
                    translucentcy = 0.2f;
                }
            }
            gridd_end();
            gridd_begin(2, 1);
            {
                add_text("Translucentcy: ");
                add_input_float(translucentcy, 0.0f, 1.0f);
            }
            gridd_end();
            gridd_begin(1, 1);
            {
                add_text("Freq --- Grain --- Oct ------- max Slope");
            }
            gridd_end();
        }
        back_bord_end();
    }
}

void recreate_terrain(Region_Alloc* region, const Application_State& app_state)
{
    recreate_graphic_pipline(region, app_state, "Syntics/res/terrain.vert.spv",
                             "Syntics/res/terrain.frag.spv", terrain_state.g_pipline,
                             size_arr(terrain_state.textures), NULL);
}

static f32 abs_f32(f32 value)
{
    return value < 0.0f ? value * -1.0f : value;
}

void update_terrain(Region_Alloc* region, VkDevice device, const Vec2& dimensions,
                    u32 semaphore_idx, f32 dt)
{
    static Vec3 pos = terrain_state.cam.position;
#if 1
    gui_update_begin(region, dimensions, semaphore_idx, dt, translucentcy);
    {
        update_gui(region, dt);
    }
    gui_update_end();

    if (!gui_focus())
#endif
    {
        update_camera(&terrain_state.cam, terrain_state.mouse_evt, dt);
    }
    static f32 speed1 = 2.0f;
    static f32 pos_x = 0.0f;
    static f32 pos_z = 0.0f;

#if 1
    Vertex_Buffer* vert = &terrain_state.g_pipline.vert_buffer;
    get_head(vert->data)->size = 0;
    // update_terrain(pos.x * 0.2f, pos.z * -0.2f);
    //   update_voxel_test(pos.x * -0.2f, pos.z * -0.2f);

    // TODO: This does not work when either pos.x or pos.y is negative.
    update_terrain_in_CPU(pos_x, pos_z);
    pos_x += speed1 * dt;
    pos_z += speed1 * dt;

    // TODO: this crasches for som reason Staging buffers seem to fuck with it
    map_copy_mem(device, &(vert->buffer_memory), vert->size_bytes, vert->data);
#endif

#if 0
    if (is_key_pressed(SYNT_LEFT_PRESSED))
    {
        pos_x -= speed2 * dt;
    }
    if (is_key_pressed(SYNT_UP_PRESSED))
    {
        pos_z -= speed1 * dt;
    }
    if (is_key_pressed(SYNT_RIGHT_PRESSED))
    {
        pos_x += speed2 * dt;
    }
    if (is_key_pressed(SYNT_DOWN_PRESSED))
    {
        pos_z += speed1 * dt;
    }

    terrain_state.cam.mvp.light_pos.x = pos_x;
    terrain_state.cam.mvp.light_pos.z = pos_z;
    terrain_state.cam.mvp.light_pos.y = 1.0f;
#endif

#if 0
    terrain_state.cam.position.x = 70.0f;
    terrain_state.cam.position.z = -45.0f;
#endif

    terrain_state.cam.mvp.view =
        view(terrain_state.cam.position,
             terrain_state.cam.position + terrain_state.cam.orientation,
             terrain_state.cam.up);
    terrain_state.cam.mvp.proj =
        perspective(radians(53.0f), dimensions.x / dimensions.y, 0.1f, 300.0f);

    update_uniform_buffers(device,
                           terrain_state.g_pipline.uniform_buffers[semaphore_idx],
                           &terrain_state.cam.mvp, sizeof(terrain_state.cam.mvp));
}

void render_terrain(VkCommandBuffer command_buffer, u32 semaphore_idx)
{
    bind_and_draw_graphics_pipline(
        command_buffer, terrain_state.g_pipline.descriptors.desc_sets[semaphore_idx],
        0, terrain_state.g_pipline.idx_buffer.curr_size, terrain_state.g_pipline);
}

void destroy_terrain(VkDevice device, u32 num_semaphores)
{
    vkDestroyPipelineLayout(device, terrain_state.g_pipline.layout, NULL);
    vkDestroyPipeline(device, terrain_state.g_pipline.pipeline, NULL);
    vkDestroyDescriptorSetLayout(device, terrain_state.g_pipline.set_layout, NULL);
    destroy_buffer(device, terrain_state.g_pipline.vert_buffer.buffer,
                   terrain_state.g_pipline.vert_buffer.buffer_memory);
    destroy_buffer(device, terrain_state.g_pipline.idx_buffer.buffer,
                   terrain_state.g_pipline.idx_buffer.buffer_memory);

    vkDestroyDescriptorPool(device, terrain_state.g_pipline.descriptors.desc_pool,
                            NULL);

    for (u32 i = 0; i < num_semaphores; i++)
    {
        destroy_buffer(device, terrain_state.g_pipline.uniform_buffers[i].buffer,
                       terrain_state.g_pipline.uniform_buffers[i].buffer_memory);
    }
    for (u32 i = 0; i < size_arr(terrain_state.textures); i++)
    {
        destroy_texture(device, terrain_state.textures[i]);
    }
}

