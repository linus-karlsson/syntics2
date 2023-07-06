#version 450

layout(location = 0) in vec4 f_color;
layout(location = 1) in vec2 f_tex_coord;
layout(location = 2) in flat float f_tex_index;

layout(location = 0) out vec4 o_color;

layout(set = 0, binding = 0) uniform sampler2D tex_sampler[3];

void main() 
{
    int idx = int(f_text_index);
    float distance = texture(msdfTexture, texCoord).r;

    float alpha = smoothstep(0.5 - 0.5 * distance, 0.5 + 0.5 * distance, 0.5);

    outColor = vec4(color.rgb, color.a * alpha);
}
