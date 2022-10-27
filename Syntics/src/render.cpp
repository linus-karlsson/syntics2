#include "render.h"
#include "region_alloc.h"
#include "buffers.h"
#include "camera.h"
#include "event_system.h"
#include "swap_chain.h"
#include "font.h"
#include "ansi_keycodes.h"
#include <string.h>
#include <vector>

namespace synt {

typedef struct Render_state
{
    Graphic_Pipline* graphic_piplines;

    VkFence* fences;
    VkSemaphore* image_semaphores;
    VkSemaphore* present_semaphores;

    VkCommandBuffer* command_buffers;

    Queues queues;

    Camera cam;
    Camera UI_cam;

    Events* mouse_evt;
    Events* key_evt;

    Texture* textures;
    Font font;

} Render_state;

static uint32 NUM_SEMAPHORES           = 1;
static uint32 SEMAPHORE_INDEX          = 0;
static Render_state render_state       = {};
static VkDevice internal_device_handle = VK_NULL_HANDLE;

static void generate_indices(Region_Alloc* region, uint32** data,
                             uint32 num_indices)
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

static void init_vert_idx(Region_Alloc* region,
                          VkPhysicalDevice physical_device,
                          VkCommandPool command_pool, uint32 num_indices,
                          Graphic_Pipline& graphic_pipline)
{
    graphic_pipline.vert_buffer.size_bytes =
        capacity_arr(graphic_pipline.vert_buffer.data) * sizeof(Vertex);

    create_vertex_buffer(internal_device_handle, physical_device, command_pool,
                         render_state.queues.graphic_queue,
                         &graphic_pipline.vert_buffer);

    graphic_pipline.idx_buffer.data =
        dyn_arrayP((*region), num_indices * 6, uint32);

    generate_indices(region, &graphic_pipline.idx_buffer.data, num_indices);

    graphic_pipline.idx_buffer.size_bytes =
        capacity_arr(graphic_pipline.idx_buffer.data) * sizeof(uint32);

    create_index_buffer(internal_device_handle, physical_device, command_pool,
                        render_state.queues.graphic_queue,
                        &graphic_pipline.idx_buffer);

    region_pop((*region), capacity_arr(graphic_pipline.idx_buffer.data), uint32,
               PERM_ARRAY);
    region_pop((*region), capacity_arr(graphic_pipline.vert_buffer.data),
               Vertex, PERM_ARRAY);

    graphic_pipline.idx_buffer.data  = NULL;
    graphic_pipline.vert_buffer.data = NULL;
}

void init_render_state(Region_Alloc* region, VkDevice device, Queues queues,
                       VkPhysicalDevice physical_device,
                       VkCommandPool command_pool,
                       const Queue_Family_Indices& q_indices,
                       uint32 num_semaphores,
                       const Swap_Chain_attrib& swap_chain)
{
    render_state.graphic_piplines = dyn_arrayP((*region), 2, Graphic_Pipline);

    create_graphics_pipeline(region, device, swap_chain.color_format,
                             swap_chain.render_pass, "Syntics/res/vert.spv",
                             "Syntics/res/frag.spv", swap_chain.extent_2D.width,
                             swap_chain.extent_2D.height,
                             &render_state.graphic_piplines[0]);

    get_head(render_state.graphic_piplines)->size++;

    create_graphics_pipeline(
        region, device, swap_chain.color_format, swap_chain.render_pass,
        "Syntics/res/gui.vert.spv", "Syntics/res/gui.frag.spv",
        swap_chain.extent_2D.width, swap_chain.extent_2D.height,
        &render_state.graphic_piplines[1]);

    get_head(render_state.graphic_piplines)->size++;
    internal_device_handle = device;

    render_state.queues = queues;

    render_state.textures = dyn_arrayP((*region), 2, Texture);

    create_texture(device, physical_device, command_pool,
                   render_state.queues.graphic_queue,
                   "Syntics/res/Arielfont.png", &render_state.textures[0]);

    get_head(render_state.textures)->size++;

    create_texture(device, physical_device, command_pool,
                   render_state.queues.graphic_queue,
                   "Syntics/res/Arielfont.png", &render_state.textures[1]);

    get_head(render_state.textures)->size++;

    render_state.font           = load_font_file("Syntics/res/Arielfont.fnt");
    render_state.font.tex_index = 1.0f;

    uint32 num_indices = text(
        region, render_state.font,
        "tool, one that {everyone} @@@@@@ can use, even people who are not\n"
        "profes-sional designers. Why? Because we are all designers in the\n"
        "sense that all of us deliberately design our lives, our rooms, and\n"
        "the way we do things. We can also design workarounds, ways of\n"
        "overcom-ing the flaws of existing devices. So, one purpose of this\n"
        "book is to give back your control over the products in your life: to\n"
        "know how to select usable and understandable ones, to know how to "
        "fix\n"
        "those that aren’t so usable or understandable. The first edition of\n"
        "the book has lived a long and healthy life. Its name was quickly\n"
        "changed to Design of Everyday Things (DOET) to make the title less\n"
        "cute and more descriptive. DOET has been read by the general public\n"
        "and by designers. It has been assigned in courses and handed out as\n"
        "required readings in many compa-nies. Now, more than twenty years\n"
        "after its release, the book is still popular. I am delighted by the\n"
        "response and by the number of people who correspond with me about "
        "it,\n"
        "who send me further examples of thoughtless, inane design, plus\n"
        "occasional examples of superb design. Many readers have told me that\n"
        "it has changed their lives, making them more sensitive to the\n"
        "problems of life and to the needs of people. Some changed their\n"
        "careers and became designers because of the book. The response has\n"
        "been amazing.",
        { 0.0f, 0.0f, 0.0f }, 800, 600,
        &render_state.graphic_piplines[0].vert_buffer.data);

    init_vert_idx(region, physical_device, command_pool, num_indices,
                  render_state.graphic_piplines[0]);

    render_state.graphic_piplines[1].vert_buffer.data =
        dyn_array_valP((*region), 0, Vertex,
                       sy({ { -1.0f, 1.0f, 0.0f },
                            { 1.0f, 1.0f, 1.0f, 1.0f },
                            { 0.0f, 0.0f },
                            1.0f },
                          { { -1.0f, -1.0f, 0.0f },
                            { 1.0f, 1.0f, 1.0f, 1.0f },
                            { 0.0f, 1.0f },
                            1.0f },
                          { { 1.0f, -1.0f, 0.0f },
                            { 1.0f, 1.0f, 1.0f, 1.0f },
                            { 1.0f, 1.0f },
                            1.0f },
                          { { 1.0f, 1.0f, 0.0f },
                            { 1.0f, 1.0f, 1.0f, 1.0f },
                            { 1.0f, 0.0f },
                            1.0f }));

    init_vert_idx(region, physical_device, command_pool, 1,
                  render_state.graphic_piplines[1]);

    NUM_SEMAPHORES = num_semaphores;

    render_state.fences = region_mallocP((*region), NUM_SEMAPHORES, VkFence);

    render_state.image_semaphores =
        region_mallocP((*region), NUM_SEMAPHORES, VkSemaphore);

    render_state.present_semaphores =
        region_mallocP((*region), NUM_SEMAPHORES, VkSemaphore);

    render_state.command_buffers =
        region_mallocP((*region), NUM_SEMAPHORES, VkCommandBuffer);

    for (uint32 i = 0; i < size_arr(render_state.graphic_piplines); i++)
    {
        render_state.graphic_piplines[i].uniform_buffers =
            region_mallocP((*region), NUM_SEMAPHORES, Uniform_Buffer);
        render_state.graphic_piplines[i].descriptors.desc_sets =
            region_mallocP((*region), NUM_SEMAPHORES, VkDescriptorSet);
    }

    for (uint32 i = 0; i < NUM_SEMAPHORES; i++)
    {
        create_fence_semaphore(device, &render_state.fences[i],
                               &render_state.image_semaphores[i],
                               &render_state.present_semaphores[i]);

        allocate_commandbuffer(device, command_pool,
                               &render_state.command_buffers[i]);

        for (uint32 j = 0; j < size_arr(render_state.graphic_piplines); j++)
        {
            render_state.graphic_piplines[j].uniform_buffers[i].size_bytes =
                (uint32)sizeof(MVP);

            create_uniform_buffer(
                device, physical_device,
                &render_state.graphic_piplines[j].uniform_buffers[i]);
        }
    }

    for (uint32 i = 0; i < size_arr(render_state.graphic_piplines); i++)
    {
        create_descriptors(
            region, device, &render_state.graphic_piplines[i].descriptors,
            NUM_SEMAPHORES, render_state.graphic_piplines[i].set_layout,
            render_state.textures, size_arr(render_state.textures),
            render_state.graphic_piplines[i].uniform_buffers);
    }

    render_state.cam.speed = 2.0f;

    render_state.cam.position    = synt::v3f(0.0f, 0.0f, 4.0f);
    render_state.cam.orientation = synt::v3f(0.0f, 0.0f, -1.0f);
    render_state.cam.mvp.model   = scale(
          rotate(mat4i(1.0f), (float)radians(1.0f), X), v3f(1.0f, 1.0f, 1.0f));
    render_state.cam.mvp.view =
        synt::view(render_state.cam.position,
                   render_state.cam.position + render_state.cam.orientation,
                   render_state.cam.up);

    render_state.UI_cam.position    = synt::v3f(0.0f, 0.0f, 1.0f);
    render_state.UI_cam.orientation = synt::v3f(0.0f, 0.0f, -1.0f);
    render_state.UI_cam.mvp.model   = scale(mat4i(1.0f), v3f(0.3f, 0.3f, 0.3f));
    render_state.UI_cam.mvp.view    = synt::view(
           render_state.UI_cam.position,
           render_state.UI_cam.position + render_state.UI_cam.orientation,
           render_state.UI_cam.up);

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
    VK_ASSERT(
        vkCreateSemaphore(device, &semaphore_info, NULL, image_semaphores));
    VK_ASSERT(
        vkCreateSemaphore(device, &semaphore_info, NULL, present_semaphores));
}

// TODO: JUST TESTING AROUND.
#if 0 
static void reconstruct_vert_idx(Region_Alloc* region,
                                 Application_State& app_state, uint16 key)
{
    vkDeviceWaitIdle(internal_device_handle);
    destroy_buffer(app_state.device, render_state.vert_buffer.buffer,
                   render_state.vert_buffer.buffer_memory);

    destroy_buffer(app_state.device, render_state.idx_buffer.buffer,
                   render_state.idx_buffer.buffer_memory);

    char letter;
    if (key == SYNT_KEY_ENTER)
        letter = '\n';
    else
        letter = (char)code_to_ascii(key);

    static std::vector<char> texting;
    if (texting.size()) texting.pop_back();
    texting.push_back(letter);
    texting.push_back('\0');

    uint32 num_indices = text(region, render_state.font, texting.data(),
                              { 400.0f, 300.0f, 0.0f }, 800, 600,
                              &render_state.vert_buffer.data);

    render_state.vert_buffer.size_bytes =
        capacity_arr(render_state.vert_buffer.data) * sizeof(Vertex);

    create_vertex_buffer(app_state.device, app_state.phy_device,
                         app_state.com_pool, render_state.queues.graphic_queue,
                         &render_state.vert_buffer);

    render_state.idx_buffer.data =
        dyn_arrayP((*region), num_indices * 6, uint32);

    generate_indices(region, &render_state.idx_buffer.data, num_indices);

    render_state.idx_buffer.size_bytes =
        capacity_arr(render_state.idx_buffer.data) * sizeof(uint32);

    create_index_buffer(app_state.device, app_state.phy_device,
                        app_state.com_pool, render_state.queues.graphic_queue,
                        &render_state.idx_buffer);

    region_pop((*region), capacity_arr(render_state.idx_buffer.data), uint32,
               PERM_ARRAY);
    region_pop((*region), capacity_arr(render_state.vert_buffer.data), Vertex,
               PERM_ARRAY);

    render_state.idx_buffer.data  = NULL;
    render_state.vert_buffer.data = NULL;
}
#endif

static void update_uniform_buffers(VkDevice device,
                                   const Uniform_Buffer& uniform_buffer,
                                   void* data, size_t size_bytes)
{
    void* transer_data;
    vkMapMemory(device, uniform_buffer.buffer_memory, 0, sizeof(MVP), 0,
                &transer_data);
    memcpy(transer_data, data, size_bytes);
    vkUnmapMemory(device, uniform_buffer.buffer_memory);
}

void render(Region_Alloc* region, Application_State& app_state, float dt)
{
    float swap_chain_width  = app_state.swap_chain.extent_2D.width;
    float swap_chain_height = app_state.swap_chain.extent_2D.height;

    static float test = 0.0f;

    vkWaitForFences(internal_device_handle, 1,
                    &render_state.fences[SEMAPHORE_INDEX], VK_TRUE, UINT64_MAX);

    uint32 image_index = 0;
    VkResult result    = vkAcquireNextImageKHR(
           internal_device_handle, app_state.swap_chain.swap_chain, UINT64_MAX,
           render_state.image_semaphores[SEMAPHORE_INDEX], VK_NULL_HANDLE,
           &image_index);

    vkResetFences(internal_device_handle, 1,
                  &render_state.fences[SEMAPHORE_INDEX]);

    update_camera(&render_state.cam, render_state.mouse_evt, dt);

    if (is_key_pressed(SYNT_E_PRESSED)) test += 60.0f * dt;
    if (is_key_pressed(SYNT_Q_PRESSED)) test -= 60.0f * dt;

    render_state.cam.mvp.proj = perspective(
        radians(53.0f), swap_chain_width / swap_chain_height, 0.1f, 100.0f);

    render_state.cam.mvp.model =
        scale(rotate(mat4i(1.0f), test * (float)radians(1.0f), X),
              v3f(1.0f, 1.0f, 1.0f));

    update_uniform_buffers(
        internal_device_handle,
        render_state.graphic_piplines[0].uniform_buffers[SEMAPHORE_INDEX],
        &render_state.cam.mvp, sizeof(render_state.cam.mvp));

    render_state.UI_cam.mvp.proj = perspective(
        radians(53.0f), swap_chain_width / swap_chain_height, 0.1f, 10.0f);

    update_uniform_buffers(
        internal_device_handle,
        render_state.graphic_piplines[1].uniform_buffers[SEMAPHORE_INDEX],
        &render_state.UI_cam.mvp, sizeof(render_state.UI_cam.mvp));

#if 0
    static bool clicked = false;
    if (is_any_key_pressed())
    {
        if (!clicked)
        {
            reconstruct_vert_idx(region, app_state,
                                 render_state.key_evt->key_evt.key);
            clicked = true;
        }
    }
    else
    {
        clicked = false;
    }
#endif

    begin_render_pass(render_state.command_buffers[SEMAPHORE_INDEX],
                      app_state.swap_chain.render_pass,
                      app_state.swap_chain.framebuffers[image_index],
                      app_state.swap_chain.extent_2D);

    for (uint32 i = 0; i < size_arr(render_state.graphic_piplines); i++)
    {
        bind_and_draw_graphics_pipline(
            render_state.command_buffers[SEMAPHORE_INDEX],
            render_state.graphic_piplines[i]
                .descriptors.desc_sets[SEMAPHORE_INDEX],
            render_state.graphic_piplines[i], true);
    }

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
        recreate_swapchain(region, &app_state, &render_state.graphic_piplines,
                           width, height);
    }

    static float sec = 0;
    sec += dt;
    if (sec >= 0.5)
    {
        // synt_LOG("(x: %f, y: %f, z:%f)\n", render_state.cam.position.x,
        //          render_state.cam.position.y, render_state.cam.position.z);
        sec = 0;
    }

    if (++SEMAPHORE_INDEX >= NUM_SEMAPHORES) SEMAPHORE_INDEX = 0;
}

void submit_and_present(VkQueue graphic_queue, VkQueue present_queue,
                        VkSemaphore image_semaphore,
                        VkSemaphore present_semaphore, VkFence fence,
                        VkCommandBuffer command_buffer,
                        VkSwapchainKHR swap_chain, uint32 image_index)
{

    VkPipelineStageFlags wait_stage =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

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
    for (uint32 i = 0; i < size_arr(render_state.graphic_piplines); i++)
    {
        vkDestroyPipelineLayout(internal_device_handle,
                                render_state.graphic_piplines[i].layout, NULL);
        vkDestroyPipeline(internal_device_handle,
                          render_state.graphic_piplines[i].pipeline, NULL);
        vkDestroyDescriptorSetLayout(
            internal_device_handle, render_state.graphic_piplines[i].set_layout,
            NULL);
        destroy_buffer(
            internal_device_handle,
            render_state.graphic_piplines[i].vert_buffer.buffer,
            render_state.graphic_piplines[i].vert_buffer.buffer_memory);
        destroy_buffer(
            internal_device_handle,
            render_state.graphic_piplines[i].idx_buffer.buffer,
            render_state.graphic_piplines[i].idx_buffer.buffer_memory);

        vkDestroyDescriptorPool(
            internal_device_handle,
            render_state.graphic_piplines[i].descriptors.desc_pool, NULL);
    }

    for (uint32 i = 0; i < NUM_SEMAPHORES; i++)
    {
        vkDestroyFence(internal_device_handle, render_state.fences[i], NULL);
        vkDestroySemaphore(internal_device_handle,
                           render_state.image_semaphores[i], NULL);
        vkDestroySemaphore(internal_device_handle,
                           render_state.present_semaphores[i], NULL);

        for (uint32 j = 0; j < size_arr(render_state.graphic_piplines); j++)
        {
            destroy_buffer(
                internal_device_handle,
                render_state.graphic_piplines[j].uniform_buffers[i].buffer,
                render_state.graphic_piplines[j]
                    .uniform_buffers[i]
                    .buffer_memory);
        }
    }

    for (uint32 i = 0; i < size_arr(render_state.textures); i++)
    {
        destroy_texture(internal_device_handle, render_state.textures[i]);
    }
}

} // namespace synt
