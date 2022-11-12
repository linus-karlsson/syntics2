#include "render.h"
#include "region_alloc.h"
#include "buffers.h"
#include "camera.h"
#include "event_system.h"
#include "swap_chain.h"
#include "font.h"
#include "ansi_keycodes.h"
#include "collision.h"
#include "file_reading.h"
#include "gui.h"
#include <stb/stb_truetype.h>
#include <msdfgen/msdfgen.h>
#include <msdfgen/msdfgen-ext.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <tiny-obj/tiny_obj_loader.h>

namespace synt {

#define MAIN_PIPELINE 0
#define UI_PIPELINE 1

static const char* OBJ_PATH = "Syntics/res/kiha32/kiha32.obj";
static const char* PNG_PATH = "Syntics/res/kiha32/1591184735691.png";

typedef struct Render_state
{
    Graphic_Pipline* g_piplines;

    VkFence* fences;
    VkSemaphore* image_semaphores;
    VkSemaphore* present_semaphores;

    VkCommandBuffer* command_buffers;

    Queues queues;

    Camera cam;

    Events* mouse_evt;
    Events* key_evt;

    Texture* textures;
    Font font;
} Render_state;

static uint32 NUM_SEMAPHORES     = 1;
static uint32 SEMAPHORE_INDEX    = 0;
static Render_state render_state = {};
static VkDevice device_handle    = VK_NULL_HANDLE;

static void load_vertices_indices(Region_Alloc* region,
                                  Graphic_Pipline* graphic_pipline, VkDevice device,
                                  VkPhysicalDevice phy_device,
                                  VkCommandPool com_pool, VkQueue graphic_queue)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, OBJ_PATH))
        synt::ERROR((warn + err).c_str());

    uint32_t sum = 0;
    for (const auto& shape : shapes)
        sum += (uint32_t)shape.mesh.indices.size();

    Vertex* vertex_buffer = dyn_array((*region), sum, Vertex, TEMP_ARRAY);
    uint32* index_buffer  = dyn_array((*region), sum, uint32, TEMP_ARRAY);

    uint32 idx = 0;
    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            synt::Vertex vertex = {};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2],
            };

            vertex.tex_coords = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
            };

            vertex.color = { 1.0f, 1.0f, 1.0f, 1.0f };

            vertex.tex_index = 0.0f;

            synt_push(vertex_buffer, vertex);
            synt_push(index_buffer, idx++);
        }
    }

    // TODO: fix small glitches.
#if 0
     Obj_Load_Attrib loader;

     loader.load_model(OBJ_PATH);

     uint32 size = size_arr(loader.indices);

     Temp_Alloc<Vertex> vertex_buffer(region, size * 3);
     Temp_Alloc<uint32> index_buffer(region, size * 3);

     uint32 idx = 0;
     for (uint32_t i = 0; i < size; i++)
    {
         for (uint32_t j = 0; j < 3; j++)
         {
             Vertex vertex = {};

            vertex.pos = loader.verts[loader.indices[i].vertex_index[j]];

            // vertex.texCoord.x =
            tex_coords[loader.indices.texture_index[i]].x;
            // vertex.texCoord.y = 1.0f -
            tex_coords[loader.indices.texture_index[i]].y;

            vertex.color = { 1.0f, 1.0f, 1.0f, 1.0f };

            vertex.tex_coords.x =
            loader.tex_coords[loader.indices[i].texture_index[j]].x;
            vertex.tex_coords.y
            =
                1.0f -
                loader.tex_coords[loader.indices[i].texture_index[j]].y;

            // printf("(x: %f, y: %f, z: %f)\n", vertex.pos.x, vertex.pos.y,
            // vertex.pos.z);

            vertex.tex_index = 0.0f;

            vertex_buffer.push_back(vertex);
            index_buffer.push_back(idx++);
        }
    }
#endif
    graphic_pipline->vert_buffer.data = vertex_buffer;
    graphic_pipline->idx_buffer.data  = index_buffer;

    graphic_pipline->vert_buffer.size_bytes =
        size_arr(vertex_buffer) * sizeof(Vertex);
    create_vertex_buffer(device, phy_device, com_pool, graphic_queue,
                         &graphic_pipline->vert_buffer);

    graphic_pipline->idx_buffer.size_bytes = size_arr(index_buffer) * sizeof(uint32);
    graphic_pipline->idx_buffer.curr_size  = size_arr(index_buffer);
    create_index_buffer(device, phy_device, com_pool, graphic_queue,
                        &graphic_pipline->idx_buffer);

    region_pop((*region), capacity_arr(index_buffer), uint32, TEMP_ARRAY);
    region_pop((*region), capacity_arr(vertex_buffer), Vertex, TEMP_ARRAY);

    graphic_pipline->vert_buffer.data = NULL;
    graphic_pipline->idx_buffer.data  = NULL;
}

static void generate_indices(Region_Alloc* region, uint32** data, uint32 num_indices)
{
    Temp_Alloc<uint32> temp(region, num_indices * 6);
    for (uint32 i = 0; i < num_indices; i++)
    {
        temp.push_back(0 + (4 * i));
        temp.push_back(1 + (4 * i));
        temp.push_back(2 + (4 * i));
        temp.push_back(2 + (4 * i));
        temp.push_back(3 + (4 * i));
        temp.push_back(0 + (4 * i));
    }
    memcpy(*data, temp.data, (num_indices * 6) * sizeof(uint32));
}

static void init_vert_idx(Region_Alloc* region, VkPhysicalDevice physical_device,
                          VkCommandPool command_pool, uint32 num_indices,
                          Graphic_Pipline& graphic_pipline)
{
    graphic_pipline.vert_buffer.size_bytes =
        capacity_arr(graphic_pipline.vert_buffer.data) * sizeof(Vertex);

    create_vertex_buffer(device_handle, physical_device, command_pool,
                         render_state.queues.graphic_queue,
                         &graphic_pipline.vert_buffer);

    graphic_pipline.idx_buffer.data = dyn_arrayP((*region), num_indices * 6, uint32);

    generate_indices(region, &graphic_pipline.idx_buffer.data, num_indices);

    graphic_pipline.idx_buffer.size_bytes =
        capacity_arr(graphic_pipline.idx_buffer.data) * sizeof(uint32);

    create_index_buffer(device_handle, physical_device, command_pool,
                        render_state.queues.graphic_queue,
                        &graphic_pipline.idx_buffer);

    region_pop((*region), capacity_arr(graphic_pipline.idx_buffer.data), uint32,
               PERM_ARRAY);

    graphic_pipline.idx_buffer.data = NULL;
}

static int32 max(int32 f, int32 s) { return (f > s) ? f : s; }

static inline Vec2 mouse_pos_to_pos(const Vec2& mouse_pos, const Vec2& window_size)
{
    // Pos from top left corner (0, 0)
    static const float x_start = -1.0f;
    static const float y_start = -1.0f;

    return Vec2((x_start + ((mouse_pos.x * 2) / window_size.x)),
                (y_start + ((mouse_pos.y * 2) / window_size.y)));
}

#if 0
    Vertex verts[4 * 6] = {};
    verts[0].pos        = { -0.5f, -0.5f, -10.9f };
    verts[0].tex_coords = { 0.0f, 0.0f };

    verts[1].pos        = { -0.5f, 0.5f, -10.9f };
    verts[1].tex_coords = { 0.0f, 1.0f };

    verts[2].pos        = { 0.5f, 0.5f, -10.9f };
    verts[2].tex_coords = { 1.0f, 1.0f };

    verts[3].pos        = { 0.5f, -0.5f, -10.9f };
    verts[3].tex_coords = { 1.0f, 0.0f };

    for (uint32 i = 0; i < 4; i++)
    {
        synt_push(render_state.graphic_piplines[UI_PIPELINE].vert_buffer.data,
                  verts[i]);
    }

#endif

void init_render_state(Region_Alloc* region, VkDevice device, Queues queues,
                       VkPhysicalDevice physical_device, VkCommandPool command_pool,
                       const Queue_Family_Indices& q_indices, uint32 num_semaphores,
                       const Swap_Chain_attrib& swap_chain)
{
    render_state.g_piplines = dyn_arrayP((*region), 2, Graphic_Pipline);

    create_graphics_pipeline(region, device, swap_chain.color_format,
                             swap_chain.render_pass, swap_chain.sample_count,
                             "Syntics/res/vert.spv", "Syntics/res/frag.spv",
                             swap_chain.extent_2D.width, swap_chain.extent_2D.height,
                             VK_CULL_MODE_NONE, &render_state.g_piplines[0]);

    get_head(render_state.g_piplines)->size++;

    device_handle = device;

    render_state.queues = queues;

    render_state.textures = dyn_arrayP((*region), 2, Texture);

    create_texture(device, physical_device, command_pool,
                   render_state.queues.graphic_queue, true, VK_FORMAT_R8G8B8A8_SRGB,
                   PNG_PATH, &render_state.textures[0]);

    get_head(render_state.textures)->size++;

    create_texture(device, physical_device, command_pool,
                   render_state.queues.graphic_queue, true, VK_FORMAT_R8G8B8A8_SRGB,
                   "Syntics/res/Arielfont.png", &render_state.textures[1]);

    get_head(render_state.textures)->size++;

    render_state.font           = load_font_file("Syntics/res/ArialSmall.fnt");
    render_state.font.tex_index = 1.0f;

#if 1
    load_vertices_indices(region, &render_state.g_piplines[MAIN_PIPELINE], device,
                          physical_device, command_pool,
                          render_state.queues.graphic_queue);

    render_state.cam.position    = synt::v3f(-7.0f, 6.0f, 11.0f);
    render_state.cam.orientation = synt::v3f(0.5f, -0.5f, -1.0f);
#endif

    NUM_SEMAPHORES = num_semaphores;

    render_state.fences = region_mallocP((*region), NUM_SEMAPHORES, VkFence);
    render_state.image_semaphores =
        region_mallocP((*region), NUM_SEMAPHORES, VkSemaphore);
    render_state.present_semaphores =
        region_mallocP((*region), NUM_SEMAPHORES, VkSemaphore);
    render_state.command_buffers =
        region_mallocP((*region), NUM_SEMAPHORES, VkCommandBuffer);

    for (uint32 i = 0; i < size_arr(render_state.g_piplines); i++)
    {
        render_state.g_piplines[i].uniform_buffers =
            region_mallocP((*region), NUM_SEMAPHORES, Uniform_Buffer);
        render_state.g_piplines[i].descriptors.desc_sets =
            region_mallocP((*region), NUM_SEMAPHORES, VkDescriptorSet);
    }

    for (uint32 i = 0; i < NUM_SEMAPHORES; i++)
    {
        create_fence_semaphore(device, &render_state.fences[i],
                               &render_state.image_semaphores[i],
                               &render_state.present_semaphores[i]);

        allocate_commandbuffer(device, command_pool,
                               &render_state.command_buffers[i]);

        for (uint32 j = 0; j < size_arr(render_state.g_piplines); j++)
        {
            render_state.g_piplines[j].uniform_buffers[i].size_bytes =
                (uint32)sizeof(MVP);

            create_uniform_buffer(device, physical_device,
                                  &render_state.g_piplines[j].uniform_buffers[i]);
        }
    }

    create_descriptors(
        region, device, &render_state.g_piplines[MAIN_PIPELINE].descriptors,
        NUM_SEMAPHORES, render_state.g_piplines[MAIN_PIPELINE].set_layout,
        render_state.textures, size_arr(render_state.textures),
        render_state.g_piplines[MAIN_PIPELINE].uniform_buffers);

    render_state.cam.speed = 2.0f;

    render_state.cam.mvp.model = scale(mat4i(1.0f), v3f(1.0f, 1.0f, 1.0f));
    render_state.cam.mvp.view =
        synt::view(render_state.cam.position,
                   render_state.cam.position + render_state.cam.orientation,
                   render_state.cam.up);

    gui_init(region, device, physical_device, command_pool,
             render_state.queues.graphic_queue, swap_chain, NUM_SEMAPHORES);

    subscribe(&render_state.mouse_evt, EVT_MOUSE);
    subscribe(&render_state.key_evt, EVT_KEY);
}

void create_fence_semaphore(VkDevice device, VkFence* fence,
                            VkSemaphore* image_semaphores,
                            VkSemaphore* present_semaphores)
{
    VkFenceCreateInfo fence_info = {};
    fence_info.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags             = VK_FENCE_CREATE_SIGNALED_BIT;

    VkSemaphoreCreateInfo semaphore_info = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VK_ASSERT(vkCreateFence(device, &fence_info, NULL, fence));
    VK_ASSERT(vkCreateSemaphore(device, &semaphore_info, NULL, image_semaphores));
    VK_ASSERT(vkCreateSemaphore(device, &semaphore_info, NULL, present_semaphores));
}

void render(Region_Alloc* region, Application_State& app_state, float dt)
{
    float swap_chain_width                = app_state.swap_chain.extent_2D.width;
    float swap_chain_height               = app_state.swap_chain.extent_2D.height;
    static const float swap_chain_width_  = swap_chain_width;
    static const float swap_chain_height_ = swap_chain_height;

    static float test = 0.0f;

    vkWaitForFences(device_handle, 1, &render_state.fences[SEMAPHORE_INDEX], VK_TRUE,
                    UINT64_MAX);

    uint32 image_index = 0;
    VkResult result    = vkAcquireNextImageKHR(
           device_handle, app_state.swap_chain.swap_chain, UINT64_MAX,
           render_state.image_semaphores[SEMAPHORE_INDEX], VK_NULL_HANDLE,
           &image_index);

    vkResetFences(device_handle, 1, &render_state.fences[SEMAPHORE_INDEX]);

    update_camera(&render_state.cam, render_state.mouse_evt, dt);

    render_state.cam.mvp.proj = perspective(
        radians(53.0f), swap_chain_width / swap_chain_height, 0.1f, 100.0f);

    update_uniform_buffers(
        device_handle, render_state.g_piplines[0].uniform_buffers[SEMAPHORE_INDEX],
        &render_state.cam.mvp, sizeof(render_state.cam.mvp));

    gui_update(region, device_handle, Vec2(swap_chain_width, swap_chain_height),
               SEMAPHORE_INDEX, dt);

    begin_render_pass(render_state.command_buffers[SEMAPHORE_INDEX],
                      app_state.swap_chain.render_pass,
                      app_state.swap_chain.framebuffers[image_index],
                      app_state.swap_chain.extent_2D);

    for (uint32 i = 0; i < size_arr(render_state.g_piplines); i++)
    {
        bind_and_draw_graphics_pipline(
            render_state.command_buffers[SEMAPHORE_INDEX],
            render_state.g_piplines[i].descriptors.desc_sets[SEMAPHORE_INDEX],
            render_state.g_piplines[i], true);
    }

    gui_render(render_state.command_buffers[SEMAPHORE_INDEX], SEMAPHORE_INDEX);

    end_render_pass(render_state.command_buffers[SEMAPHORE_INDEX]);

    submit_and_present(render_state.queues.graphic_queue,
                       render_state.queues.present_queue,
                       render_state.image_semaphores[SEMAPHORE_INDEX],
                       render_state.present_semaphores[SEMAPHORE_INDEX],
                       render_state.fences[SEMAPHORE_INDEX],
                       render_state.command_buffers[SEMAPHORE_INDEX],
                       app_state.swap_chain.swap_chain, image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        uint16 width, height;
        get_window_size(&width, &height);
        recreate_swapchain(region, &app_state, &render_state.g_piplines, width,
                           height);
    }

    if (++SEMAPHORE_INDEX >= NUM_SEMAPHORES) SEMAPHORE_INDEX = 0;
}

void submit_and_present(VkQueue graphic_queue, VkQueue present_queue,
                        VkSemaphore image_semaphore, VkSemaphore present_semaphore,
                        VkFence fence, VkCommandBuffer command_buffer,
                        VkSwapchainKHR swap_chain, uint32 image_index)
{

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit_info         = {};
    submit_info.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount   = 1;
    submit_info.pWaitSemaphores      = &image_semaphore;
    submit_info.pWaitDstStageMask    = &wait_stage;
    submit_info.commandBufferCount   = 1;
    submit_info.pCommandBuffers      = &command_buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores    = &present_semaphore;

    VK_ASSERT(vkQueueSubmit(graphic_queue, 1, &submit_info, fence));

    VkPresentInfoKHR present_info   = {};
    present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = &present_semaphore;
    present_info.swapchainCount     = 1;
    present_info.pSwapchains        = &swap_chain;
    present_info.pImageIndices      = &image_index;

    vkQueuePresentKHR(present_queue, &present_info);
}

void destroy_render_state()
{
    for (uint32 i = 0; i < size_arr(render_state.g_piplines); i++)
    {
        vkDestroyPipelineLayout(device_handle, render_state.g_piplines[i].layout,
                                NULL);
        vkDestroyPipeline(device_handle, render_state.g_piplines[i].pipeline, NULL);
        vkDestroyDescriptorSetLayout(device_handle,
                                     render_state.g_piplines[i].set_layout, NULL);
        destroy_buffer(device_handle, render_state.g_piplines[i].vert_buffer.buffer,
                       render_state.g_piplines[i].vert_buffer.buffer_memory);
        destroy_buffer(device_handle, render_state.g_piplines[i].idx_buffer.buffer,
                       render_state.g_piplines[i].idx_buffer.buffer_memory);

        vkDestroyDescriptorPool(
            device_handle, render_state.g_piplines[i].descriptors.desc_pool, NULL);
    }

    for (uint32 i = 0; i < NUM_SEMAPHORES; i++)
    {
        vkDestroyFence(device_handle, render_state.fences[i], NULL);
        vkDestroySemaphore(device_handle, render_state.image_semaphores[i], NULL);
        vkDestroySemaphore(device_handle, render_state.present_semaphores[i], NULL);

        for (uint32 j = 0; j < size_arr(render_state.g_piplines); j++)
        {
            destroy_buffer(
                device_handle, render_state.g_piplines[j].uniform_buffers[i].buffer,
                render_state.g_piplines[j].uniform_buffers[i].buffer_memory);
        }
    }

    for (uint32 i = 0; i < size_arr(render_state.textures); i++)
    {
        destroy_texture(device_handle, render_state.textures[i]);
    }

    destroy_gui(device_handle, NUM_SEMAPHORES);
}

} // namespace synt
