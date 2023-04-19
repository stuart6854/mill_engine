#version 450

layout(location = 0) in vec2 in_texCoord;
layout(location = 1) in vec3 in_color;

layout(location = 0) out vec4 out_fragColor;

void main()
{
    out_fragColor = vec4(in_color, 1.0);
}