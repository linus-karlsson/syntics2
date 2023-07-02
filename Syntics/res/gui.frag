#version 450

layout(location = 0) in vec4 f_color;
layout(location = 1) in vec2 f_tex_coord;
layout(location = 2) in flat float f_tex_index;

layout(location = 0) out vec4 o_color;

layout(set = 0, binding = 1) uniform sampler2D tex_sampler[2];

float median(float r, float g, float b) 
{
    return max(min(r, g), min(max(r, g), b));
}

// Nvidia research on hash alpha testing
float hash_2D(vec2 indd)
{
    return fract(1.0e4 * sin(17.0 * indd.x + 0.1 * indd.y) * (0.1 + abs(sin(13.0 * indd.y + indd.x))));
}

float hash_3D(vec3 indd)
{
    return hash_2D(vec2(hash_2D(indd.xy), indd.z)); 
}

float random( vec2 p )
{
     return fract(sin(dot(p.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

void main() 
{
    int idx = int(f_tex_index);
    vec4 f_texture = texture(tex_sampler[idx], f_tex_coord);

    o_color = vec4(f_texture) * f_color;
}
