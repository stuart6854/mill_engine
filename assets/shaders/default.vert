#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_color;

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 0) uniform GlobalData
{
    float time;
} u_globals;

layout(set = 1, binding = 0) uniform SceneData
{
    mat4 camera_proj;
    mat4 camera_view;
} u_scene;

layout(push_constant) uniform PushBlock
{
    mat4 transform;
} u_pushConsts;

void main()
{
    vec4 world_position = u_pushConsts.transform * vec4(in_position, 1.0);
    world_position.y += sin(u_globals.time * 0.1);

    gl_Position = u_scene.camera_proj * u_scene.camera_view * world_position;
    out_color = in_color;
}