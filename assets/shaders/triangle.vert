#version 450

// layout(location = 0) in vec3 in_position;
// layout(location = 1) in vec4 in_color;

layout(location = 0) out vec3 out_color;

void main()
{
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

    gl_Position = vec4(pos[gl_VertexIndex], 1.0);
    out_color = color[gl_VertexIndex];
}