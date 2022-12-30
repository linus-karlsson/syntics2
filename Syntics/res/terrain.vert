#version 450 core

layout(location = 0) in vec4 i_pos;
layout(location = 1) in vec4 i_color;
layout(location = 2) in vec2 i_tex_coords;
layout(location = 3) in float i_tex_index;

layout(location = 0) out vec4 f_color;
layout(location = 1) out vec2 f_tex_coord;
layout(location = 2) out flat float f_tex_index;

layout(binding = 0) uniform ModelViewProjection {
    mat4 model;
    mat4 view;
    mat4 proj;
} MVP;

float fade(float t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float lerp(float a, float b, float t)
{
    return a + (t * (b - a));
}

float random( vec2 p )
{
     return fract(sin(dot(p.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

float noise(float x, float y)
{
    vec2 thing = vec2(x, y);

    vec2 lv = fract(thing);
    vec2 id = floor(thing);
    
    lv = lv*lv*(3.-2.*lv);
    
    float bl = random(id);
    float br = random(id+vec2(1,0));
    float b = mix(bl, br, lv.x);
    
    float tl = random(id+vec2(0,1));
    float tr = random(id+vec2(1,1));
    float t = mix(tl, tr, lv.x);
    
    return mix(b, t, lv.y);
}

float perlin2d(float x, float y, float freq, float gain, int oct)
{
    float amp    = gain;
    float result = 0.0;
    float max_    = 0.0;

    for(int i = 0; i < oct; i++)
    {
        max_ += 256.0 * amp;
        result += noise(x * freq, y * freq) * amp;
        amp *= gain;
        freq *= 2.0;
    }

    return result / max_;
}

void main() 
{
    float perlin =   perlin2d(i_tex_coords.x, i_tex_coords.y, 0.41, 0.6, 2) * 40.0;
    vec4 final_pos = vec4(i_pos.x, perlin * 100.0, i_pos.z, i_pos.w);

    gl_Position = MVP.proj * MVP.view * MVP.model * final_pos;
    gl_PointSize = 10.0;
    f_color = vec4(perlin);
    f_color.a = 1.0;
    f_tex_coord = i_tex_coords;
    f_tex_index = i_tex_index;
}

