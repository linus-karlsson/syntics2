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

    Window_Handle* win_handles;

} Notebook;
