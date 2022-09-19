#version 450

layout(location = 0) in vec4 f_color;
layout(location = 1) in vec2 f_tex_coord;

layout(location = 0) out vec4 o_color;

layout(binding = 1) uniform sampler2D tex_sampler;

void main() 
{
    o_color = texture(tex_sampler, f_tex_coord) * f_color;
}
