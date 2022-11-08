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
    Graphic_Pipline* graphic_piplines;

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

    Camera UI_cam;
    Rect* UI_rects;
    Texture* UI_textures;

} Render_state;

static uint32 NUM_SEMAPHORES           = 1;
static uint32 SEMAPHORE_INDEX          = 0;
static Render_state render_state       = {};
static VkDevice internal_device_handle = VK_NULL_HANDLE;

static void load_vertices_indices(Region_Alloc* region,
                                  Graphic_Pipline* graphic_pipline,
                                  VkDevice device, VkPhysicalDevice phy_device,
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

    graphic_pipline->idx_buffer.size_bytes =
        size_arr(index_buffer) * sizeof(uint32);
    create_index_buffer(device, phy_device, com_pool, graphic_queue,
                        &graphic_pipline->idx_buffer);

    region_pop((*region), capacity_arr(index_buffer), uint32, TEMP_ARRAY);
    region_pop((*region), capacity_arr(vertex_buffer), Vertex, TEMP_ARRAY);

    graphic_pipline->vert_buffer.data = NULL;
    graphic_pipline->idx_buffer.data  = NULL;
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

    graphic_pipline.idx_buffer.data = NULL;
}

static Rect quad(Vertex** vertices, const Vec3& pos, const Vec2& size,
                 const Vec4& color, float tex_index)
{
    Vertex verts[4] = { { { pos.x, pos.y, pos.z },
                          { color.x, color.y, color.z, color.w },
                          { 0.0f, 0.0f },
                          tex_index },
                        { { pos.x, pos.y + size.y, pos.z },
                          { color.x, color.y, color.z, color.w },
                          { 0.0f, 1.0f },
                          tex_index },
                        { { pos.x + size.x, pos.y + size.y, pos.z },
                          { color.x, color.y, color.z, color.w },
                          { 1.0f, 1.0f },
                          tex_index },
                        { { pos.x + size.x, pos.y, pos.z },
                          { color.x, color.y, color.z, color.w },
                          { 1.0f, 0.0f },
                          tex_index } };

    for (uint32 i = 0; i < 4; i++)
    {
        synt_push((*vertices), verts[i]);
    }

    Rect out;
    out.pos.x = pos.x;
    out.pos.y = pos.y;
    out.size  = size;
    return out;
}

static int32 max(int32 f, int32 s) { return (f > s) ? f : s; }

static inline Vec2 mouse_pos_to_pos(const Vec2& mouse_pos,
                                    const Vec2& window_size)
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

unsigned char* render_font(const char* word)
{
    long size;
    unsigned char* fontBuffer;

    FILE* fontFile = fopen("Syntics/res/aakar-medium.ttf", "rb");
    fseek(fontFile, 0, SEEK_END);
    size = ftell(fontFile);       /* how long is the file ? */
    fseek(fontFile, 0, SEEK_SET); /* reset */

    fontBuffer = (unsigned char*)malloc(size);

    fread(fontBuffer, size, 1, fontFile);
    fclose(fontFile);

    /* prepare font */
    stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, fontBuffer, 0))
    {
        printf("failed\n");
    }

    int b_w = 512; /* bitmap width */
    int b_h = 128; /* bitmap height */
    int l_h = 12;  /* line height */

    /* create a bitmap for the phrase */
    unsigned char* bitmap =
        (unsigned char*)calloc(b_w * b_h, sizeof(unsigned char));

    /* calculate font scaling */
    float scale = stbtt_ScaleForPixelHeight(&info, l_h);

    int x = 0;

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

    ascent  = roundf(ascent * scale);
    descent = roundf(descent * scale);

    int i;
    for (i = 0; i < strlen(word); ++i)
    {
        /* how wide is this character */
        int ax;
        int lsb;
        stbtt_GetCodepointHMetrics(&info, word[i], &ax, &lsb);
        /* (Note that each Codepoint call has an alternative Glyph version which
         * caches the work required to lookup the character word[i].) */

        /* get bounding box for character (may be offset to account for chars
         * that dip above or below the line) */
        int c_x1, c_y1, c_x2, c_y2;
        stbtt_GetCodepointBitmapBox(&info, word[i], scale, scale, &c_x1, &c_y1,
                                    &c_x2, &c_y2);

        /* compute y (different characters have different heights) */
        int y = ascent + c_y1;

        /* render character (stride and offset is important here) */
        int byteOffset = x + roundf(lsb * scale) + (y * b_w);
        stbtt_MakeCodepointBitmap(&info, bitmap + byteOffset, c_x2 - c_x1,
                                  c_y2 - c_y1, b_w, scale, scale, word[i]);

        /* advance x */
        x += roundf(ax * scale);

        /* add kerning */
        int kern;
        kern = stbtt_GetCodepointKernAdvance(&info, word[i], word[i + 1]);
        x += roundf(kern * scale);
    }

    free(fontBuffer);
    return bitmap;
}

void init_render_state(Region_Alloc* region, VkDevice device, Queues queues,
                       VkPhysicalDevice physical_device,
                       VkCommandPool command_pool,
                       const Queue_Family_Indices& q_indices,
                       uint32 num_semaphores,
                       const Swap_Chain_attrib& swap_chain)
{
    render_state.graphic_piplines = dyn_arrayP((*region), 2, Graphic_Pipline);

    create_graphics_pipeline(
        region, device, swap_chain.color_format, swap_chain.render_pass,
        swap_chain.sample_count, "Syntics/res/vert.spv", "Syntics/res/frag.spv",
        swap_chain.extent_2D.width, swap_chain.extent_2D.height,
        VK_CULL_MODE_NONE, &render_state.graphic_piplines[0]);

    get_head(render_state.graphic_piplines)->size++;

    create_graphics_pipeline(
        region, device, swap_chain.color_format, swap_chain.render_pass,
        swap_chain.sample_count, "Syntics/res/gui.vert.spv",
        "Syntics/res/gui.frag.spv", swap_chain.extent_2D.width,
        swap_chain.extent_2D.height, VK_CULL_MODE_NONE,
        &render_state.graphic_piplines[1]);

    get_head(render_state.graphic_piplines)->size++;

    internal_device_handle = device;

    render_state.queues = queues;

    render_state.textures = dyn_arrayP((*region), 2, Texture);

    create_texture(device, physical_device, command_pool,
                   render_state.queues.graphic_queue, VK_FORMAT_R8G8B8A8_SRGB,
                   PNG_PATH, &render_state.textures[0]);

    get_head(render_state.textures)->size++;

    create_texture(device, physical_device, command_pool,
                   render_state.queues.graphic_queue, VK_FORMAT_R8G8B8A8_SRGB,
                   "Syntics/res/Times.png", &render_state.textures[1]);

    get_head(render_state.textures)->size++;

    render_state.font           = load_font_file("Syntics/res/Times.fnt");
    render_state.font.tex_index = 1.0f;

#if 1
    load_vertices_indices(region, &render_state.graphic_piplines[MAIN_PIPELINE],
                          device, physical_device, command_pool,
                          render_state.queues.graphic_queue);

    render_state.cam.position    = synt::v3f(-7.0f, 6.0f, 11.0f);
    render_state.cam.orientation = synt::v3f(0.5f, -0.5f, -1.0f);
#endif

    render_state.UI_textures = dyn_arrayP((*region), 2, Texture);

    create_texture(device, physical_device, command_pool,
                   render_state.queues.graphic_queue, VK_FORMAT_R8G8B8A8_SRGB,
                   "Syntics/res/white-color-solid-background-1920x1080.png",
                   &render_state.UI_textures[0]);

    get_head(render_state.UI_textures)->size++;

    unsigned char* bitmap =
        render_font("this is a test and it will go for ever");
    int b_w = 512; /* bitmap width */
    int b_h = 128; /* bitmap height */

    render_state.UI_textures[1].size_bytes = (uint32)b_w * b_h;
    render_state.UI_textures[1].width      = (uint32)b_w;
    render_state.UI_textures[1].height     = (uint32)b_h;
    render_state.UI_textures[1].mip_map_lvl =
        (uint32)(std::floor(std::log2(max(b_w, b_h)))) + 1;

    create_texture(device, physical_device, command_pool,
                   render_state.queues.graphic_queue, VK_FORMAT_R8_UNORM,
                   &render_state.UI_textures[1], bitmap);
    free(bitmap);
#if 0
    create_texture(device, physical_device, command_pool,
                   render_state.queues.graphic_queue, "Syntics/res/Times.png",
                   &render_state.UI_textures[1]);
#endif

    get_head(render_state.UI_textures)->size++;

    uint32 num_ui_rects = 1;

    render_state.UI_rects = dyn_arrayP((*region), num_ui_rects, Rect);

    render_state.graphic_piplines[UI_PIPELINE].vert_buffer.data =
        dyn_arrayP((*region), (num_ui_rects)*4, Vertex);

    const float swap_chain_width  = swap_chain.extent_2D.width;
    const float swap_chain_height = swap_chain.extent_2D.height;

    synt_push(render_state.UI_rects,
              quad(&render_state.graphic_piplines[UI_PIPELINE].vert_buffer.data,
                   { -1.0f, -1.0f, 0.0f }, Vec2(0.5f, 0.5f),
                   Vec4(0.2f, 0.2f, 0.2f, 1.0f), 1.0f));

    init_vert_idx(region, physical_device, command_pool, num_ui_rects,
                  render_state.graphic_piplines[UI_PIPELINE]);

    region_pop((*region),
               capacity_arr(
                   render_state.graphic_piplines[UI_PIPELINE].vert_buffer.data),
               Vertex, PERM_ARRAY);

    render_state.graphic_piplines[UI_PIPELINE].vert_buffer.data = NULL;

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

    create_descriptors(
        region, device,
        &render_state.graphic_piplines[MAIN_PIPELINE].descriptors,
        NUM_SEMAPHORES, render_state.graphic_piplines[MAIN_PIPELINE].set_layout,
        render_state.textures, size_arr(render_state.textures),
        render_state.graphic_piplines[MAIN_PIPELINE].uniform_buffers);

    create_descriptors(
        region, device, &render_state.graphic_piplines[UI_PIPELINE].descriptors,
        NUM_SEMAPHORES, render_state.graphic_piplines[UI_PIPELINE].set_layout,
        render_state.UI_textures, size_arr(render_state.textures),
        render_state.graphic_piplines[UI_PIPELINE].uniform_buffers);

    render_state.cam.speed = 2.0f;

    render_state.cam.mvp.model = scale(mat4i(1.0f), v3f(1.0f, 1.0f, 1.0f));
    render_state.cam.mvp.view =
        synt::view(render_state.cam.position,
                   render_state.cam.position + render_state.cam.orientation,
                   render_state.cam.up);

    render_state.UI_cam.position    = synt::v3f(0.0f, 0.0f, 0.0f);
    render_state.UI_cam.orientation = synt::v3f(0.0f, 0.0f, -1.0f);
    render_state.UI_cam.mvp.model   = mat4i(1.0f);
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

    bool ui_hit = false;
    // TODO: For some reason, this fucks up the recreation of the swap_chain.
#if 1
    for (uint32 i = 0; i < size_arr(render_state.UI_rects); i++)
    {
        ui_hit =
            ui_hit ||
            point_in_rect(
                mouse_pos_to_pos(
                    Vec2(
                        (float)render_state.mouse_evt->mouse_evt.move_evt.pos_x,
                        (float)
                            render_state.mouse_evt->mouse_evt.move_evt.pos_y),
                    Vec2(swap_chain_width, swap_chain_height)),
                render_state.UI_rects[i]);
    }
#endif
    // TODO: End

#if 0
    if (render_state.key_evt->activated)
    {
        void* transfer_data = NULL;
        VK_ASSERT(vkMapMemory(
            internal_device_handle,
            render_state.graphic_piplines[UI_PIPELINE]
                .vert_buffer.buffer_memory,
            0,
            render_state.graphic_piplines[UI_PIPELINE].vert_buffer.size_bytes,
            0, &transfer_data));
        memcpy(
            transfer_data,
            (void*)render_state.graphic_piplines[UI_PIPELINE].vert_buffer.data,
            (size_t)render_state.graphic_piplines[UI_PIPELINE]
                .vert_buffer.size_bytes);
        vkUnmapMemory(internal_device_handle,
                      render_state.graphic_piplines[UI_PIPELINE]
                          .vert_buffer.buffer_memory);
    }
#endif
    if (ui_hit)
    {
    }
    else
    {
        update_camera(&render_state.cam, render_state.mouse_evt, dt);
    }

    render_state.cam.mvp.proj = perspective(
        radians(53.0f), swap_chain_width / swap_chain_height, 0.1f, 100.0f);

    render_state.cam.mvp.model = scale(mat4i(1.0f), v3f(1.0f, 1.0f, 1.0f));

    update_uniform_buffers(
        internal_device_handle,
        render_state.graphic_piplines[0].uniform_buffers[SEMAPHORE_INDEX],
        &render_state.cam.mvp, sizeof(render_state.cam.mvp));

    // TODO: Because vulkan is flipped this results in the oposite for y axis :|
    render_state.UI_cam.mvp.proj = perspective(
        radians(53.0f), swap_chain_width / swap_chain_height, 0.1f, 100.0f);

    update_uniform_buffers(
        internal_device_handle,
        render_state.graphic_piplines[1].uniform_buffers[SEMAPHORE_INDEX],
        &render_state.UI_cam.mvp, sizeof(render_state.UI_cam.mvp));

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
    for (uint32 i = 0; i < size_arr(render_state.UI_textures); i++)
    {
        destroy_texture(internal_device_handle, render_state.UI_textures[i]);
    }
}

} // namespace synt
