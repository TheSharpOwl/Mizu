struct PSInput // same as GS output
{
    float4 position : SV_POSITION;
    float4 color : COLOR; // TODO remove color area from passing it to the shaders to save time
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
    // calculate the area of the triangle:
    float fArea = 0.5f * abs(input[0].position.x * (input[1].position.y - input[2].position.y)
        + input[1].position.x * (input[2].position.y - input[0].position.y)
        + input[2].position.x * (input[0].position.y - input[1].position.y));

    // map fArea between [0,1]
    float a = 3.12505290e-06;
    float b = 0.00319999829;

    fArea = (fArea - a) / (b - a);

    for (int i = 0; i < 3; ++i)
    {
        output.position = input[i].position;
        output.color = float4(fArea, 1.f, fArea, input[i].color.w);
        outStream.Append(output);
    }

}
