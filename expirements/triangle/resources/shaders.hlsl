struct PSInput // same as GS output
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

struct VSInput // same as GS output
{
    float4 position : SV_POSITION;
};

VSInput VSMain(float4 position : POSITION)
{
    VSInput result;
    result.position = position;
    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}

[maxvertexcount(3)]
void GSMain(triangle VSInput input[3], inout TriangleStream<PSInput> outStream)
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
        // using position to represent color from this stage to save memory
        output.position = input[i].position;
        output.color = float4(fArea, 1.f, fArea, 1.f);
        outStream.Append(output);
    }

}
