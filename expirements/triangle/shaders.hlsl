struct PSInput // same as GS output
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PSInput VSMain(float4 position : POSITION, float4 color : COLOR)
{
    PSInput result;

    result.position = position;
    result.color = color;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}

[maxvertexcount(3)]
void GSMain(triangle PSInput input[3], inout TriangleStream<PSInput> outStream)
{
    PSInput output;
    [unroll(3)]
    for (int i = 0; i < 3; ++i)
    {
        output.position = input[i].position;
        output.color = input[i].color;
        outStream.Append(output);
    }
}