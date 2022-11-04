#version 450

layout(location = 0) in vec4 f_color;
layout(location = 1) in vec2 f_tex_coord;
layout(location = 2) in flat float f_tex_index;

layout(location = 0) out vec4 o_color;

layout(binding = 1) uniform sampler2D tex_sampler[2];

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

float screenPxRange(int idx) {
    vec2 unitRange = vec2(0.43)/vec2(textureSize(tex_sampler[idx], 0));
    vec2 screenTexSize = vec2(1.0)/fwidth(f_tex_coord);
    return max(0.5*dot(unitRange, screenTexSize), 1.0);
}

void main() 
{
    int idx = int(f_tex_index);
    vec4 f_texture = texture(tex_sampler[idx], f_tex_coord);
    if(idx == 0)
    {
        float sd = median(f_texture.r, f_texture.g, f_texture.b);
        float screen_px_distance = screenPxRange(idx) * (sd - 0.5);
        float opacity = clamp(screen_px_distance + 0.5, 0.0, 1.0);
        o_color = mix(vec4(0.0,0.0,0.0,1.0), f_color, opacity); 
    }
    else
    {
        if(f_texture.a < 0.18)
            discard;
        o_color = f_texture * f_color;
    }
}
