struct VSInput
{
    float4 position : POSITION;
    float4 color : COLOR;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PSInput VSMain(uint vertex_id : SV_VERTEXID)
{
    static VSInput vertices[3] = {
        { float4(0, 0.4, 0, 1.0), float4(1.0, 0.0, 0.0, 1.0) },
        { float4(0.4, -0.4, 0, 1.0), float4(0.0, 1.0, 0.0, 1.0) },
        { float4(-0.4, -0.4, 0, 1.0), float4(0.0, 0.0, 1.0, 1.0) },
    };

    PSInput output;
    output.position = vertices[vertex_id].position;
    output.color = vertices[vertex_id].color;

    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}