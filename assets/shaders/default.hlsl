struct VSInput
{
    float4 position : POSITION;
    float4 color : COLOR;
};

cbuffer ConstantBuffer : register(b0)
{
    // NOTE: Matrices must be row_major to work with GLM
    row_major float4x4 projection;
    row_major float4x4 view;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PSInput VSMain(VSInput input)
{
    float4 position = input.position;
    // position = mul(position, world);
    position = mul(position, view);
    position = mul(position, projection);

    PSInput output;
    output.position = position;
    output.color = input.color;

    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}