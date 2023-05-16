#version 450

layout(location = 0) in vec3 i_pos;
layout(location = 1) in vec3 i_normal;
layout(location = 2) in vec2 i_tex_coords;
layout(location = 3) in vec4 i_color;
layout(location = 4) in float i_tex_index;

layout(location = 0) out vec4 f_color;
layout(location = 1) out vec2 f_tex_coord;
layout(location = 2) out flat float f_tex_index;

layout(binding = 0) uniform ModelViewProjection {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 light_dir;
} MVP;

layout(push_constant) uniform Lightning {
    vec3 pos;
} Light; 

void main() 
{
    vec3 light_dir = vec3(0.5, 1.0, 0.0);
    float intensity = dot(i_normal, Light.pos);
    vec3 final_color = vec3(i_color) * intensity;

    gl_Position = MVP.proj * MVP.view * MVP.model * vec4(i_pos, 1.0);
    f_color = vec4(final_color, 1.0f);
    f_tex_coord = i_tex_coords;
    f_tex_index = i_tex_index;
}

