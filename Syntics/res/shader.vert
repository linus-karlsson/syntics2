#version 450

layout(location = 0) out vec4 f_color;


void main() 
{
    gl_Position = vec4(1.0, 1.0, 1.0, 1.0);
    f_color = vec4(1.0, 0.0, 0.0, 1.0);
}

