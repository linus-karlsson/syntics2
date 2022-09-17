#include "vulkan_api.h"
#include "buffers.h"
#include "instance_device.h"
#include "region_alloc.h"
#include "swap_chain.h"
#include "render.h"
#include "obj_load.h"

namespace synt {

static Application_State* internal_handle = NULL;
static bool INITIALIZED                   = false;

void init_vulkan(Region_Alloc* region, Application_State* app_state, uint32 height,
                 uint32 width)
{
    if (INITIALIZED) ERROR("Already initialized vulkan");

    Obj_Load_Attrib loader;

    loader.load_model("Syntics/res/kiha32.obj");

    uint32 size = size_arr(loader.indices);

    app_state->vert_buffer.data = dyn_array((*region), size * 3, Vertex, PERM_ARRAY);
    app_state->idx_buffer.data  = dyn_array((*region), size * 3, uint32, PERM_ARRAY);

    for (uint32_t i = 0; i < size; i++)
        for (uint32_t j = 0; j < 3; j++)
        {
            Vertex vertex = {};

            vertex.pos = loader.verts[loader.indices[i].vertex_index[j]];

            // vertex.texCoord.x = tex_coords[loader.indices.texture_index[i]].x;
            // vertex.texCoord.y = 1.0f - tex_coords[loader.indices.texture_index[i]].y;

            vertex.color = { 0.0f, 0.0f, 0.0f, 1.0f };

            // printf("(x: %f, y: %f, z: %f)\n", vertex.pos.x, vertex.pos.y,
            // vertex.pos.z);

            synt_push(app_state->vert_buffer.data, vertex);
            synt_push(app_state->idx_buffer.data, size_arr(app_state->idx_buffer.data));
        }

    app_state->vert_buffer.size_bytes =
        size_arr(app_state->vert_buffer.data) * sizeof(Vertex);
    app_state->idx_buffer.size_bytes =
        size_arr(app_state->idx_buffer.data) * sizeof(uint32);

    init_instance(region);
    if (VALIDATIONS_ENABLE) init_debug_messenger();

    create_surface(get_platform_state(), &app_state->surface);

    pick_physical_device(region, get_instance(), app_state->surface,
                         &app_state->phy_device, &app_state->q_indices);

    create_logical_device(app_state->phy_device, app_state->q_indices,
                          &app_state->device);

    create_vertex_buffer(app_state->device, app_state->phy_device,
                         &app_state->vert_buffer);

    create_index_buffer(app_state->device, app_state->phy_device, &app_state->idx_buffer);

    create_swapchain(region, app_state->phy_device, app_state->device, app_state->surface,
                     width, height, app_state->q_indices, &app_state->swap_chain);

    get_swapchain_images(region, app_state->device, &app_state->swap_chain);

    create_graphics_pipeline(
        region, app_state->device, app_state->swap_chain.color_format,
        "Syntics/res/vert.spv", "Syntics/res/frag.spv",
        app_state->swap_chain.extent_2D.width, app_state->swap_chain.extent_2D.height,
        &app_state->swap_chain.graphic_pipline);

    app_state->swap_chain.img_views = region_malloc(
        (*region), app_state->swap_chain.num_images, VkImageView, PERM_MALLOC);

    app_state->swap_chain.framebuffers = region_malloc(
        (*region), app_state->swap_chain.num_images, VkFramebuffer, PERM_MALLOC);

    for (uint32 i = 0; i < app_state->swap_chain.num_images; i++)
    {
        create_image_view(app_state->device, app_state->swap_chain.images[i],
                          VK_IMAGE_VIEW_TYPE_2D, app_state->swap_chain.color_format,
                          VK_IMAGE_ASPECT_COLOR_BIT, &app_state->swap_chain.img_views[i]);

        create_frame_buffer(
            app_state->device, app_state->swap_chain.graphic_pipline.render_pass,
            app_state->swap_chain.extent_2D, app_state->swap_chain.img_views[i],
            &app_state->swap_chain.framebuffers[i]);
    }

    create_command_pool(app_state->device,
                        app_state->q_indices.indices[GRAPHICS_QUEUE_IDX],
                        &app_state->com_pool);

    init_render_state(
        region, app_state->device, app_state->phy_device, app_state->com_pool,
        app_state->swap_chain.graphic_pipline.set_layout, app_state->q_indices, 2);

    internal_handle = app_state;
    INITIALIZED     = true;
}

void destroy_vulkan()
{
    vkDeviceWaitIdle(internal_handle->device);

    for (uint32 i = 0; i < internal_handle->swap_chain.num_images; i++)
    {
        vkDestroyFramebuffer(internal_handle->device,
                             internal_handle->swap_chain.framebuffers[i], NULL);
        vkDestroyImageView(internal_handle->device,
                           internal_handle->swap_chain.img_views[i], NULL);
    }
    vkDestroySwapchainKHR(internal_handle->device, internal_handle->swap_chain.swap_chain,
                          NULL);

    vkDestroyRenderPass(internal_handle->device,
                        internal_handle->swap_chain.graphic_pipline.render_pass, NULL);
    vkDestroyPipelineLayout(internal_handle->device,
                            internal_handle->swap_chain.graphic_pipline.layout, NULL);
    vkDestroyPipeline(internal_handle->device,
                      internal_handle->swap_chain.graphic_pipline.pipeline, NULL);
    vkDestroyDescriptorSetLayout(internal_handle->device,
                                 internal_handle->swap_chain.graphic_pipline.set_layout,
                                 NULL);

    destroy_render_state();

    vkDestroyCommandPool(internal_handle->device, internal_handle->com_pool, NULL);

    destroy_buffer(internal_handle->device, internal_handle->vert_buffer.buffer,
                   internal_handle->vert_buffer.buffer_memory);
    destroy_buffer(internal_handle->device, internal_handle->idx_buffer.buffer,
                   internal_handle->idx_buffer.buffer_memory);

    vkDestroyDevice(internal_handle->device, NULL);

    vkDestroySurfaceKHR(get_instance(), internal_handle->surface, NULL);
    destroy_instance();
}

} // namespace synt
