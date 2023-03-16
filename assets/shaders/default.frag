#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec2 in_uv;
layout(location = 1) in vec4 in_color;

layout(location = 0) out vec4 out_fragColor;

layout(set = 0, binding = 1) uniform sampler2D u_textures[];

layout(push_constant) uniform PushBlock
{
    mat4 transform;
    uint textureId;
} u_pushConsts;

void main()
{
    out_fragColor = texture(u_textures[u_pushConsts.textureId], in_uv);
}