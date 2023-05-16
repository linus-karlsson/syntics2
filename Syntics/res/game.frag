#version 450

layout(location = 0) in vec4 f_color;
layout(location = 1) in vec2 f_tex_coord;
layout(location = 2) in flat float f_tex_index;

layout(location = 0) out vec4 o_color;

layout(binding = 1) uniform sampler2D tex_sampler[2];

void main() 
{
    int idx = int(f_tex_index);
    vec4 f_texture = texture(tex_sampler[idx], f_tex_coord);
    o_color = f_texture * f_color;
}
