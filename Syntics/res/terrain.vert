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
    vec3 light_pos;
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

float change_range(float value, float max_val, float min_val)
{
    return (value - min_val) / (max_val - min_val);
}

float cal_attenuation(float constant, float linear, float quadratic, float distance_)
{
    return 1.0 / (constant + linear * distance_ + quadratic * (distance_ * distance_));
}

void main() 
{
    //float HEIGHT = 50.0;
    //float FREQ = 0.41;
    //float GRAIN = 0.55;
    //int OCT = 2;

    //float perlin =   perlin2d(i_tex_coords.x, i_tex_coords.y, FREQ, GRAIN, OCT) * HEIGHT;
    //vec3 final_pos = vec3(i_pos.x, perlin * 80.0, i_pos.z);

    //float perlin1 =   perlin2d(i_tex_coords.x, i_tex_coords.y + 0.1, FREQ, GRAIN, OCT) * HEIGHT;
    //float perlin2 =   perlin2d(i_tex_coords.x + 0.1, i_tex_coords.y, FREQ, GRAIN, OCT) * HEIGHT;
    //vec3 neighbour_pos1 = vec3(i_pos.x, perlin1 * 80.0, i_pos.z - 0.5);
    //vec3 neighbour_pos2 = vec3(i_pos.x + 0.5, perlin2 * 80.0, i_pos.z);

    //vec3 side1 = neighbour_pos1 - final_pos;
    //vec3 side2 = neighbour_pos2 - final_pos;

    //vec3 normal = normalize(cross(side2, side1));

    //vec3 test = vec3(i_tex_coords.x, final_pos.y , i_tex_coords.y);

    //float dis = length(MVP.light_pos - test);
    //float attenuation = cal_attenuation(1.0, 0.12, 0.032, dis); 


    vec3 normal = vec3(i_color);
    vec3 up = vec3(0.0f, 1.0f, 0.0f);

    vec3 normal_world_space = normalize(mat3(MVP.model) * normal);

    vec3 light_dir = normalize(MVP.light_pos);
    float intensity = dot(normal_world_space, light_dir);

    vec3 flat_surface_color = vec3(0.35f, 0.678f, 0.09f);
    vec3 steep_surface_color = vec3(0.47f, 0.38f, 0.086f);
    float slope = acos(dot(normal, up));
    vec3 final_color = mix(flat_surface_color, steep_surface_color, slope) * intensity;

    gl_Position = MVP.proj * MVP.view * MVP.model * vec4(i_pos);
    gl_PointSize = 10.0;
    f_color = vec4(final_color, 1.0f);
    f_color.a = 1.0;
    f_tex_coord = i_tex_coords;
    f_tex_index = i_tex_index;
}

