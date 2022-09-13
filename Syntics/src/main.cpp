#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "syntics.h"

int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        synt::set_log(false);
        synt::set_log_alloc(false);
    }

    synt::Linux_Platform xcb = {};
    synt::Region_Alloc region;

    VkInstance instance = VK_NULL_HANDLE;

    VkPhysicalDevice physical_device     = VK_NULL_HANDLE;
    VkDevice device                      = VK_NULL_HANDLE;
    VkQueue graphic_queue                = VK_NULL_HANDLE;
    synt::Queue_Family_Indices q_indices = {};

    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VkCommandPool command_pool     = VK_NULL_HANDLE;
    VkCommandBuffer command_buffer = VK_NULL_HANDLE;

    synt::Swap_Chain_attrib swap_chain;

    VkRenderPass render_pass = VK_NULL_HANDLE;

    synt::init_region(&region, 1000000);
    synt::init_platform(&xcb, 800, 600);
    synt::init_events(&region, 1);

    synt::create_instance(&region, &instance);
    synt::get_surface(instance, xcb, &surface);
    synt::pick_physical_device(&region, instance, surface, &physical_device,
                               &q_indices);
    synt::create_logical_device(physical_device, q_indices, &device);
    vkGetDeviceQueue(device, q_indices.indices[GRAPHICS_QUEUE_IDX], 0,
                     &graphic_queue);

    const uint32 num_semaphores = 2;
    VkFence* fences =
        region_malloc(&region, num_semaphores, VkFence, synt::PERM_MALLOC);
    VkSemaphore* semaphores =
        region_malloc(&region, num_semaphores, VkSemaphore, synt::PERM_MALLOC);

    for (uint32 i = 0; i < num_semaphores; i++)
    {
        synt::create_fence_semaphore(device, &fences[i], &semaphores[i]);
    }
    synt::create_swapchain(&region, physical_device, device, surface, xcb.width,
                           xcb.height, q_indices, &swap_chain);
    synt::create_render_pass(device, swap_chain.color_format, &render_pass);
    synt::get_swapchain_images(&region, device, &swap_chain);

    swap_chain.img_views = region_malloc(&region, swap_chain.num_images, VkImageView,
                                         synt::PERM_MALLOC);

    swap_chain.framebuffers = region_malloc(&region, swap_chain.num_images,
                                            VkFramebuffer, synt::PERM_MALLOC);

    for (uint32 i = 0; i < swap_chain.num_images; i++)
    {
        synt::create_image_view(device, swap_chain.images[i], VK_IMAGE_VIEW_TYPE_2D,
                                swap_chain.color_format, VK_IMAGE_ASPECT_COLOR_BIT,
                                &swap_chain.img_views[i]);

        synt::create_frame_buffer(device, render_pass, swap_chain.extent_2D,
                                  swap_chain.img_views[i],
                                  &swap_chain.framebuffers[i]);
    }

    synt::Graphic_Pipline graphic_pipline;

    synt::create_graphics_pipeline(&region, device, swap_chain.color_format,
                                   "Syntics/res/vert.spv", "Syntics/res/frag.spv",
                                   swap_chain.extent_2D.width,
                                   swap_chain.extent_2D.height, &graphic_pipline);

    synt::create_command_pool(device, q_indices.indices[GRAPHICS_QUEUE_IDX],
                              &command_pool);

    synt::allocate_commandbuffer(device, command_pool, &command_buffer);

    synt::print_region(region);

    synt::Events* evt;
    synt::subscribe(&evt, synt::EVT_KEY);

    bool running = true;
    while (running)
    {

        uint32 image_index = 0;

        synt::poll_events();
        if (evt->activated)
        {
            if (evt->key_evt.key == SYNT_KEY_Q) running = false;
        }
    }

    for (uint32 i = 0; i < swap_chain.num_images; i++)
    {
        vkDestroyFramebuffer(device, swap_chain.framebuffers[i], NULL);
        vkDestroyImageView(device, swap_chain.img_views[i], NULL);
    }
    vkDestroySwapchainKHR(device, swap_chain.swap_chain, NULL);
    vkDestroyRenderPass(device, render_pass, NULL);

    for (uint32 i = 0; i < num_semaphores; i++)
    {
        vkDestroyFence(device, fences[i], NULL);
        vkDestroySemaphore(device, semaphores[i], NULL);
    }
    vkDestroyCommandPool(device, command_pool, NULL);

    vkDestroyDevice(device, NULL);

    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyInstance(instance, NULL);

    synt::free_region(&region);

    synt_LOG("\nComplete!\n");

    return 0;
}
