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
    gl_Position = MVP.proj * MVP.view * MVP.model * i_pos;
    gl_PointSize = 10.0;
    f_color = i_color;
    f_color.a = 1.0;
    f_tex_coord = i_tex_coords;
    f_tex_index = i_tex_index;
}

