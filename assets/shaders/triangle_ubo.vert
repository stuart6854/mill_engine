#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texCoord;
layout(location = 2) in vec3 in_color;

layout(location = 0) out vec2 out_texCoord;
layout(location = 1) out vec3 out_color;

layout(set = 0, binding = 0) uniform CameraUniforms
{
    mat4 projection;
    mat4 view;
} u_camera;

layout (push_constant) uniform PushBlock
{
    mat4 world;
} u_consts;

void main()
{
    /*
    const vec3 pos[3] = vec3[3](
        vec3(1, 1, 0),
        vec3(-1, 1, 0),
        vec3(0, -1, 0)
    );
    const vec3 color[3] = vec3[3](
        vec3(1, 0, 0),
        vec3(0, 1, 0),
        vec3(0, 0, 1)
    );

    gl_Position = u_consts.proj * u_consts.view * u_consts.world * vec4(pos[gl_VertexIndex], 1.0);
    out_color = color[gl_VertexIndex];
    */

    gl_Position = u_camera.projection * u_camera.view * u_consts.world * vec4(in_position, 1.0);
    out_texCoord = in_texCoord;
    out_color = in_color;
}