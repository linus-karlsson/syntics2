#include "syntic_app.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <chrono>
#include <string.h>
#include "syntics.h"

namespace synt {

static void uint_to_string(char* buffer, uint32 len_buffer, uint32 i)
{
    const char* numbers = "0123456789";

    uint32 n = i;
    uint32 c = 0;
    do
    {
        c++;
    } while (n /= 10);

    assert(len_buffer >= c);

    n = c;
    do
    {
        (buffer)[--c] = numbers[i % 10];
    } while (i /= 10);

    (buffer)[n] = '\0';
}

// - source x.org
static void change_title(const synt::Linux_Platform& xcb, const char* title,
                         uint32 len)
{
    xcb_change_property(xcb.connection, XCB_PROP_MODE_REPLACE, xcb.window,
                        XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, len, title);
    xcb_flush(xcb.connection);
}

typedef struct Instance_Debug_Creation
{
    Region_Alloc* region              = NULL;
    Linux_Platform* xcb               = NULL;
    VkSurfaceKHR* surface             = VK_NULL_HANDLE;
    VkPhysicalDevice* physical_device = VK_NULL_HANDLE;
    Queue_Family_Indices* q_indices   = NULL;
} Instance_Debug_Creation;

static void instance_debug_creation(Instance_Debug_Creation* creation)
{
    static VkInstance instance                      = VK_NULL_HANDLE;
    static VkSurfaceKHR internal_surface_handle     = VK_NULL_HANDLE;
    static VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;

    if (creation != NULL)
    {
        if (instance != VK_NULL_HANDLE) ERROR("instance already created");

        synt::create_instance(creation->region, &instance);
        if (synt::VALIDATIONS_ENABLE)
            debug_messenger = synt::init_debug_messenger(instance);

        synt::get_surface(instance, *creation->xcb, creation->surface);
        synt::pick_physical_device(creation->region, instance, *creation->surface,
                                   creation->physical_device, creation->q_indices);

        internal_surface_handle = *creation->surface;
    }
    else
    {
        if (!instance || !internal_surface_handle || !debug_messenger)
            ERROR("Trying to destroy instance and more without creating it first.");

        vkDestroySurfaceKHR(instance, internal_surface_handle, NULL);
        destroy_debug_messenger(instance, debug_messenger, NULL);
        vkDestroyInstance(instance, NULL);
    }
}

static void destroy_instance_surface() { instance_debug_creation(NULL); }

static void render(Region_Alloc* region, VkDevice device, VkCommandPool command_pool,
                   const Queue_Family_Indices* q_indices,
                   const Swap_Chain_attrib* swap_chain,
                   const Vertex_Buffer* vertex_buffer,
                   const Index_Buffer* index_buffer)
{
    static bool first_time             = true;
    static const uint32 num_semaphores = 2;
    static uint32 semaphore_index      = 0;

    static VkFence* fences                 = NULL;
    static VkSemaphore* image_semaphores   = NULL;
    static VkSemaphore* present_semaphores = NULL;

    static VkCommandBuffer* command_buffers = NULL;

    static VkQueue graphic_queue = VK_NULL_HANDLE;
    static VkQueue present_queue = VK_NULL_HANDLE;

    if (!first_time && region == NULL)
    {
        for (uint32 i = 0; i < num_semaphores; i++)
        {
            vkDestroyFence(device, fences[i], NULL);
            vkDestroySemaphore(device, image_semaphores[i], NULL);
            vkDestroySemaphore(device, present_semaphores[i], NULL);
        }
        return;
    }
    if (first_time)
    {
        vkGetDeviceQueue(device, q_indices->indices[GRAPHICS_QUEUE_IDX], 0,
                         &graphic_queue);

        vkGetDeviceQueue(device, q_indices->indices[GRAPHICS_QUEUE_IDX], 0,
                         &present_queue);

        fences             = region_mallocP((*region), num_semaphores, VkFence);
        image_semaphores   = region_mallocP((*region), num_semaphores, VkSemaphore);
        present_semaphores = region_mallocP((*region), num_semaphores, VkSemaphore);

        command_buffers = region_mallocP((*region), num_semaphores, VkCommandBuffer);

        for (uint32 i = 0; i < num_semaphores; i++)
        {
            synt::create_fence_semaphore(device, &fences[i], &image_semaphores[i],
                                         &present_semaphores[i]);

            synt::allocate_commandbuffer(device, command_pool, &command_buffers[i]);
        }

        synt::print_region(*region);
    }

    vkWaitForFences(device, 1, &fences[semaphore_index], VK_TRUE, UINT64_MAX);

    uint32 image_index = 0;
    VK_ASSERT(vkAcquireNextImageKHR(device, swap_chain->swap_chain, UINT64_MAX,
                                    image_semaphores[semaphore_index],
                                    VK_NULL_HANDLE, &image_index));

    vkResetFences(device, 1, &fences[semaphore_index]);

    synt::record_execute_commandbuffer(
        command_buffers[semaphore_index], swap_chain->framebuffers[image_index],
        swap_chain->extent_2D, vertex_buffer->buffer, index_buffer->buffer,
        synt::size_arr(index_buffer->data), swap_chain->graphic_pipline);

    synt::submit_and_present(
        graphic_queue, present_queue, image_semaphores[semaphore_index],
        present_semaphores[semaphore_index], fences[semaphore_index],
        command_buffers[semaphore_index], swap_chain->swap_chain, image_index);

    if (++semaphore_index >= num_semaphores) semaphore_index = 0;
    first_time = false;
}

static void destroy_sync_objects(VkDevice device)
{
    render(NULL, device, VK_NULL_HANDLE, NULL, NULL, NULL, NULL);
}

void run_app(int argc, char* argv[])
{
    if (argc > 1)
    {
        synt::set_log(false);
        synt::set_log_alloc(false);
    }

    synt::Linux_Platform xcb = {};
    synt::Region_Alloc region;

    synt::init_region(&region, 1000000);
    synt::init_platform(&xcb, 800, 600);
    synt::init_events(&region, 1);

    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice device                  = VK_NULL_HANDLE;

    synt::Queue_Family_Indices q_indices = {};

    VkSurfaceKHR surface = VK_NULL_HANDLE;

    Instance_Debug_Creation instance_creation;
    instance_creation.region          = &region;
    instance_creation.xcb             = &xcb;
    instance_creation.surface         = &surface;
    instance_creation.physical_device = &physical_device;
    instance_creation.q_indices       = &q_indices;

    instance_debug_creation(&instance_creation);

    VkCommandPool command_pool       = VK_NULL_HANDLE;
    VkCommandBuffer* command_buffers = VK_NULL_HANDLE;

    synt::Swap_Chain_attrib swap_chain = {};

    Vertex_Buffer vertex_buffer = {};
    vertex_buffer.data =
        dyn_array_valP(region, 0, Vertex,
                       sy({ { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
                          { { 0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
                          { { 0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
                          { { -0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }, ));
    vertex_buffer.size_bytes = size_arr(vertex_buffer.data) * sizeof(Vertex);

    Index_Buffer index_buffer = {};
    index_buffer.data = dyn_array_valP(region, 0, uint32, sy(0, 1, 2, 2, 3, 0));
    index_buffer.size_bytes = size_arr(index_buffer.data) * sizeof(uint32);

    synt::create_logical_device(physical_device, q_indices, &device);

    synt::create_swapchain(&region, physical_device, device, surface, xcb.width,
                           xcb.height, q_indices, &swap_chain);

    synt::get_swapchain_images(&region, device, &swap_chain);

    synt::create_graphics_pipeline(
        &region, device, swap_chain.color_format, "Syntics/res/vert.spv",
        "Syntics/res/frag.spv", swap_chain.extent_2D.width,
        swap_chain.extent_2D.height, &swap_chain.graphic_pipline);

    swap_chain.img_views =
        region_malloc(region, swap_chain.num_images, VkImageView, synt::PERM_MALLOC);

    swap_chain.framebuffers = region_malloc(region, swap_chain.num_images,
                                            VkFramebuffer, synt::PERM_MALLOC);

    for (uint32 i = 0; i < swap_chain.num_images; i++)
    {
        synt::create_image_view(device, swap_chain.images[i], VK_IMAGE_VIEW_TYPE_2D,
                                swap_chain.color_format, VK_IMAGE_ASPECT_COLOR_BIT,
                                &swap_chain.img_views[i]);

        synt::create_frame_buffer(device, swap_chain.graphic_pipline.render_pass,
                                  swap_chain.extent_2D, swap_chain.img_views[i],
                                  &swap_chain.framebuffers[i]);
    }

    synt::create_command_pool(device, q_indices.indices[GRAPHICS_QUEUE_IDX],
                              &command_pool);

    synt::create_vertex_buffer(device, physical_device, &vertex_buffer);
    synt::create_index_buffer(device, physical_device, &index_buffer);

    synt::Events* evt;
    synt::subscribe(&evt, synt::EVT_KEY);

    change_title(xcb, "FPS: 0", 7);

    const uint32 frames_to_count = 50;

    float delta_time = 0.0f;
    float sec        = 0.0f;
    float sec2       = 0.0f;
    uint32 fps       = 0;
    uint32 frames    = 0;
    bool running     = true;
    auto start2      = std::chrono::high_resolution_clock::now();
    while (running)
    {
        auto start = std::chrono::high_resolution_clock::now();

        if (frames == 0) start2 = std::chrono::high_resolution_clock::now();
        if (frames++ >= frames_to_count)
        {
            auto end2  = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float, std::chrono::seconds::period>(
                             end2 - start2)
                             .count();

            fps    = frames_to_count / time;
            frames = 0;
        }
        sec += delta_time;
        sec2 += delta_time;
        if (sec2 >= 2.0f)
        {
            synt::print_region(region);
            sec2 = 0;
        }
        if (sec >= 0.1f)
        {
            char title[30] = "FPS: ";
            uint_to_string(title + 5, 30, fps);
            change_title(xcb, title, strlen(title));
            sec = 0;
        }

        render(&region, device, command_pool, &q_indices, &swap_chain,
               &vertex_buffer, &index_buffer);

        synt::poll_events();
        if (evt->activated)
        {
            if (evt->key_evt.key == SYNT_KEY_Q) running = false;
        }

        auto end = std::chrono::high_resolution_clock::now();
        delta_time =
            std::chrono::duration<float, std::chrono::seconds::period>(end - start)
                .count();
    }
    vkDeviceWaitIdle(device);

    for (uint32 i = 0; i < swap_chain.num_images; i++)
    {
        vkDestroyFramebuffer(device, swap_chain.framebuffers[i], NULL);
        vkDestroyImageView(device, swap_chain.img_views[i], NULL);
    }
    vkDestroySwapchainKHR(device, swap_chain.swap_chain, NULL);

    vkDestroyRenderPass(device, swap_chain.graphic_pipline.render_pass, NULL);
    vkDestroyPipelineLayout(device, swap_chain.graphic_pipline.layout, NULL);
    vkDestroyPipeline(device, swap_chain.graphic_pipline.pipeline, NULL);

    destroy_sync_objects(device);

    vkDestroyCommandPool(device, command_pool, NULL);

    destroy_buffer(device, vertex_buffer.buffer, vertex_buffer.buffer_memory);

    destroy_buffer(device, index_buffer.buffer, index_buffer.buffer_memory);

    vkDestroyDevice(device, NULL);

    destroy_instance_surface();

    synt_LOG("\nComplete!\n");
}

} // namespace synt
