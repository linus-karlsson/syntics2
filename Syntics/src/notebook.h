#pragma once

typedef struct Notebook
{
    VkPipelineLayout pipeline_layout;
    VkDescriptorSetLayout descriptor_set_layout;

    Buffer* uniform_buffers;
    Descriptors descriptors;

    VkPipeline triangle_list_pipeline;

    Texture* textures;

    V2 dimensions;
    VP vp;

    Vertex_Index_Buffer vert_idx;

    Window_Handle* win_handles;

    Font_TTF font;
} Notebook;
