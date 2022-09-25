#version 450

layout(location = 0) in vec4 f_color;
layout(location = 1) in vec2 f_tex_coord;
layout(location = 2) in flat float f_tex_index;

layout(location = 0) out vec4 o_color;

layout(binding = 1) uniform sampler2D tex_sampler[2];

void main() 
{
    int i = int(f_tex_index);
    o_color = texture(tex_sampler[i], f_tex_coord) * f_color;
}
