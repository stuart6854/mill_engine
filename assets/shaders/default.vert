#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_color;

layout(location = 0) out vec4 out_color;

layout(push_constant) uniform PushBlock
{
    mat4 projection;
    mat4 view;
} u_pushConsts;

void main()
{
    gl_Position = u_pushConsts.projection * u_pushConsts.view * vec4(in_position, 1.0);
    out_color = in_color;
}