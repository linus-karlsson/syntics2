#version 450 core

layout(location = 0) in vec4 vColor;
layout(location = 1) in vec2 vPosition;
layout(location = 2) in vec2 vTexCoord;
layout(location = 3) in float vTexIndex;

layout(location = 0) out vec4 fColor;
layout(location = 1) out vec2 fTexCoord;
layout(location = 2) out flat float fTexIndex;

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
    gl_Position = VP.proj * VP.view * Model.model * vec4(vPosition, 0.0f, 1.0f);
    fColor = vColor;
    fTexCoord = vTexCoord;
    fTexIndex = vTexIndex;
}
