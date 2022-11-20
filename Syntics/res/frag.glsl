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
    float gamma = 2.2;
    vec3 final_color = pow(f_color.rgb, vec3(1.0/gamma));
    o_color = f_texture * vec4(final_color, f_color.a);
}
