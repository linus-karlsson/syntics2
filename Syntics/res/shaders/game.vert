#version 450

layout(location = 0) in vec3 i_pos;
layout(location = 1) in vec3 i_normal;
layout(location = 2) in vec2 i_tex_coords;
layout(location = 3) in vec4 i_color;
layout(location = 4) in float i_tex_index;

layout(location = 0) out vec4 f_color;
layout(location = 1) out vec2 f_tex_coord;
layout(location = 2) out flat float f_tex_index;
layout(location = 3) out vec3 f_normal;

layout(binding = 0) uniform ViewProjection
{
    mat4 view;
    mat4 proj;
}
VP;

layout(push_constant) uniform ModelMatrix
{
    mat4 model;
}
Model;

void main()
{
    vec3 light_dir = vec3(0.5f, 1.0f, 0.5f);
    float intensity = dot(i_normal, light_dir);
    vec3 final_color = i_color.rgb;// * intensity;

    gl_Position = VP.proj * VP.view * Model.model * vec4(i_pos, 1.0);
    f_color = vec4(final_color, i_color.a);
    f_tex_coord = i_tex_coords;
    f_tex_index = i_tex_index;
    f_normal = i_normal;
}

