struct VSOutput // same as PS input
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VSOutput main(float3 position : POSITION, float4 color : COLOR)
{
    VSOutput result;
    result.position = float4(position, 1.f);
    result.color = color;

    return result;
}
