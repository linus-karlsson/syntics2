#include "vulkan_api.h"
#include "buffers.h"
#include "instance_device.h"
#include "swap_chain.h"
#include "render.h"
#include "obj_load.h"
#include "math/vectors.h"
#include <stdlib.h>
#include <vector>
#include <string.h>
#include <tiny-obj/tiny_obj_loader.h>
#include "hash.h"

namespace synt {

static Application_State* internal_handle = NULL;
static bool INITIALIZED                   = false;

static const char* OBJ_PATH = "Syntics/res/kiha32/kiha32.obj";
static const char* PNG_PATH = "Syntics/res/kiha32/1591184735691.png";

static void load_vertices_indices(Region_Alloc* region,
                                  Application_State* app_state,
                                  VkQueue graphic_queue)
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

    Temp_Alloc<Vertex> vertex_buffer(region, sum * 2);
    Temp_Alloc<uint32> index_buffer(region, sum * 2);

    uint32 idx = 0;
    for (uint32 i = 0; i < 2; i++)
    {
        for (const auto& shape : shapes)
        {
            for (const auto& index : shape.mesh.indices)
            {
                synt::Vertex vertex = {};

                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0] +
                        (float)i * 20.0f,
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2],
                };

                vertex.tex_coords = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
                };

                vertex.color = { 1.0f, 1.0f, 1.0f, 1.0f };

                vertex.tex_index = (float)i;

                vertex_buffer.push_back(vertex);
                index_buffer.push_back(idx++);
            }
        }
    }

    // TODO: fix small glitches.
    // Obj_Load_Attrib loader;

    // loader.load_model(OBJ_PATH);

    // uint32 size = size_arr(loader.indices);

    // Temp_Alloc<Vertex> vertex_buffer(region, size * 3);
    // Temp_Alloc<uint32> index_buffer(region, size * 3);

    // uint32 idx = 0;
    // for (uint32_t i = 0; i < size; i++)
    //{
    //     for (uint32_t j = 0; j < 3; j++)
    //     {
    //         Vertex vertex = {};

    //        vertex.pos = loader.verts[loader.indices[i].vertex_index[j]];

    //        // vertex.texCoord.x =
    //        tex_coords[loader.indices.texture_index[i]].x;
    //        // vertex.texCoord.y = 1.0f -
    //        tex_coords[loader.indices.texture_index[i]].y;

    //        vertex.color = { 1.0f, 1.0f, 1.0f, 1.0f };

    //        vertex.tex_coords.x =
    //        loader.tex_coords[loader.indices[i].texture_index[j]].x;
    //        vertex.tex_coords.y
    //        =
    //            1.0f -
    //            loader.tex_coords[loader.indices[i].texture_index[j]].y;

    //        // printf("(x: %f, y: %f, z: %f)\n", vertex.pos.x, vertex.pos.y,
    //        // vertex.pos.z);

    //        vertex.tex_index = 0.0f;

    //        vertex_buffer.push_back(vertex);
    //        index_buffer.push_back(idx++);
    //    }
    //}

    app_state->vert_buffer.data = vertex_buffer.data;
    app_state->idx_buffer.data  = index_buffer.data;

    app_state->vert_buffer.size_bytes = vertex_buffer.size() * sizeof(Vertex);
    create_vertex_buffer(app_state->device, app_state->phy_device,
                         app_state->com_pool, graphic_queue,
                         &app_state->vert_buffer);

    app_state->idx_buffer.size_bytes = index_buffer.size() * sizeof(uint32);
    create_index_buffer(app_state->device, app_state->phy_device,
                        app_state->com_pool, graphic_queue,
                        &app_state->idx_buffer);

    app_state->vert_buffer.data = NULL;
    app_state->idx_buffer.data  = NULL;
}

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

void init_vulkan(Region_Alloc* region, Application_State* app_state,
                 uint32 width, uint32 height)
{
    if (INITIALIZED) ERROR("Already initialized vulkan");

    init_instance(region);
    if (VALIDATIONS_ENABLE) init_debug_messenger();

    create_surface(get_platform_state(), &app_state->surface);

    pick_physical_device(region, get_instance(), app_state->surface,
                         &app_state->phy_device, &app_state->q_indices);

    create_logical_device(app_state->phy_device, app_state->q_indices,
                          &app_state->device);

    Queues queue = {};
    vkGetDeviceQueue(app_state->device,
                     app_state->q_indices.indices[GRAPHICS_QUEUE_IDX], 0,
                     &queue.graphic_queue);

    vkGetDeviceQueue(app_state->device,
                     app_state->q_indices.indices[GRAPHICS_QUEUE_IDX], 0,
                     &queue.present_queue);

    create_command_pool(app_state->device,
                        app_state->q_indices.indices[GRAPHICS_QUEUE_IDX],
                        &app_state->com_pool);

    // load_vertices_indices(region, app_state, queue.graphic_queue);

    app_state->textures = dyn_arrayP((*region), 2, Texture);

    create_texture(app_state->device, app_state->phy_device,
                   app_state->com_pool, queue.graphic_queue, PNG_PATH,
                   &app_state->textures[0]);

    get_head(app_state->textures)->size++;

    create_texture(app_state->device, app_state->phy_device,
                   app_state->com_pool, queue.graphic_queue,
                   "Syntics/res/cube.jpg", &app_state->textures[1]);

    get_head(app_state->textures)->size++;

    app_state->vert_buffer.data =
        dyn_array_valP((*region), 0, Vertex,
                       sy({ { -1.0f, -1.0f, 0.0f },
                            { 1.0f, 1.0f, 1.0f, 1.0f },
                            { 0.0f, 0.0f },
                            1.0f },
                          { { 1.0f, -1.0f, 0.0f },
                            { 1.0f, 1.0f, 1.0f, 1.0f },
                            { 1.0f, 0.0f },
                            1.0f },
                          { { 1.0f, 1.0f, 0.0f },
                            { 1.0f, 1.0f, 1.0f, 1.0f },
                            { 1.0f, 1.0f },
                            1.0f },
                          { { -1.0f, 1.0f, 0.0f },
                            { 1.0f, 1.0f, 1.0f, 1.0f },
                            { 0.0f, 1.0f },
                            1.0f },
                          { { 2.0f, -1.0f, 0.0f },
                            { 1.0f, 1.0f, 1.0f, 1.0f },
                            { 0.0f, 0.0f },
                            1.0f },
                          { { 4.0f, -1.0f, 0.0f },
                            { 1.0f, 1.0f, 1.0f, 1.0f },
                            { 1.0f, 0.0f },
                            1.0f },
                          { { 4.0f, 1.0f, 0.0f },
                            { 1.0f, 1.0f, 1.0f, 1.0f },
                            { 1.0f, 1.0f },
                            1.0f },
                          { { 2.0f, 1.0f, 0.0f },
                            { 1.0f, 1.0f, 1.0f, 1.0f },
                            { 0.0f, 1.0f },
                            1.0f }));

    app_state->vert_buffer.size_bytes =
        capacity_arr(app_state->vert_buffer.data) * sizeof(Vertex);

    create_vertex_buffer(app_state->device, app_state->phy_device,
                         app_state->com_pool, queue.graphic_queue,
                         &app_state->vert_buffer);

    uint32 num_indices         = 2;
    app_state->idx_buffer.data = dyn_arrayP((*region), num_indices * 6, uint32);

    generate_indices(region, &app_state->idx_buffer.data, num_indices);

    app_state->idx_buffer.size_bytes =
        capacity_arr(app_state->idx_buffer.data) * sizeof(uint32);

    create_index_buffer(app_state->device, app_state->phy_device,
                        app_state->com_pool, queue.graphic_queue,
                        &app_state->idx_buffer);

    region_pop((*region), capacity_arr(app_state->idx_buffer.data), uint32,
               PERM_ARRAY);
    region_pop((*region), capacity_arr(app_state->vert_buffer.data), Vertex,
               PERM_ARRAY);

    app_state->idx_buffer.data  = NULL;
    app_state->vert_buffer.data = NULL;

    // create_texture(app_state->device, app_state->phy_device, 350, 200,
    //                app_state->com_pool, queue.graphic_queue,
    //                &app_state->texture);

    create_swapchain(region, app_state->phy_device, app_state->device,
                     app_state->surface, width, height, app_state->q_indices,
                     &app_state->swap_chain);

    create_depth_image(app_state->device, app_state->phy_device,
                       app_state->swap_chain.extent_2D, &app_state->depth_img);

    get_swapchain_images(region, app_state->device, &app_state->swap_chain);

    create_graphics_pipeline(region, app_state->device,
                             app_state->swap_chain.color_format,
                             "Syntics/res/vert.spv", "Syntics/res/frag.spv",
                             app_state->swap_chain.extent_2D.width,
                             app_state->swap_chain.extent_2D.height,
                             &app_state->swap_chain.graphic_pipline);

    app_state->swap_chain.img_views =
        dyn_arrayP((*region), app_state->swap_chain.num_images, VkImageView);

    app_state->swap_chain.framebuffers =
        dyn_arrayP((*region), app_state->swap_chain.num_images, VkFramebuffer);

    for (uint32 i = 0; i < app_state->swap_chain.num_images; i++)
    {
        create_image_view(
            app_state->device, app_state->swap_chain.images[i],
            VK_IMAGE_VIEW_TYPE_2D, app_state->swap_chain.color_format,
            VK_IMAGE_ASPECT_COLOR_BIT, &app_state->swap_chain.img_views[i]);

        create_frame_buffer(app_state->device,
                            app_state->swap_chain.graphic_pipline.render_pass,
                            app_state->swap_chain.extent_2D,
                            app_state->swap_chain.img_views[i],
                            app_state->depth_img.img_view,
                            &app_state->swap_chain.framebuffers[i]);
    }

    app_state->num_semaphores = 2;
    init_render_state(
        region, app_state->device, queue, app_state->phy_device,
        app_state->com_pool, app_state->swap_chain.graphic_pipline.set_layout,
        app_state->textures, app_state->q_indices, app_state->num_semaphores);

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
    vkDestroySwapchainKHR(internal_handle->device,
                          internal_handle->swap_chain.swap_chain, NULL);

    vkDestroyRenderPass(internal_handle->device,
                        internal_handle->swap_chain.graphic_pipline.render_pass,
                        NULL);
    vkDestroyPipelineLayout(internal_handle->device,
                            internal_handle->swap_chain.graphic_pipline.layout,
                            NULL);
    vkDestroyPipeline(internal_handle->device,
                      internal_handle->swap_chain.graphic_pipline.pipeline,
                      NULL);
    vkDestroyDescriptorSetLayout(
        internal_handle->device,
        internal_handle->swap_chain.graphic_pipline.set_layout, NULL);

    destroy_render_state();

    vkDestroyCommandPool(internal_handle->device, internal_handle->com_pool,
                         NULL);

    destroy_buffer(internal_handle->device, internal_handle->vert_buffer.buffer,
                   internal_handle->vert_buffer.buffer_memory);
    destroy_buffer(internal_handle->device, internal_handle->idx_buffer.buffer,
                   internal_handle->idx_buffer.buffer_memory);

    for (uint32 i = 0; i < size_arr(internal_handle->textures); i++)
        destroy_texture(internal_handle->device, internal_handle->textures[i]);

    destroy_image(internal_handle->device, internal_handle->depth_img);

    vkDestroyDevice(internal_handle->device, NULL);

    vkDestroySurfaceKHR(get_instance(), internal_handle->surface, NULL);
    destroy_instance();
}

} // namespace synt
