#version 450

layout(location = 0) in vec4 f_color;
layout(location = 1) in vec2 f_tex_coord;
layout(location = 2) in flat float f_tex_index;

layout(location = 0) out vec4 o_color;

layout(binding = 1) uniform sampler2D tex_sampler[2];

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main() 
{
    int idx = int(f_tex_index);
    vec4 f_texture = texture(tex_sampler[idx], f_tex_coord);
    if(f_texture.a < 0.2)
        discard;
    o_color = vec4(f_texture) * f_color;
}
