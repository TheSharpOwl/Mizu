struct Input // same as GS output
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

Input VSMain(float4 position : POSITION, float4 color : COLOR)
{
    Input result;
    result.position = position;
    result.color = color;

    return result;
}

float4 PSMain(Input input) : SV_TARGET
{
    return input.color;
}

//[maxvertexcount(3)]
//void GSMain(triangle Input input[3], inout TriangleStream<Input> outStream)
//{
//    PSInput output;
//    [unroll(3)]
//    // calculate the area of the triangle:
//    float fArea = 0.5f * abs(input[0].position.x * (input[1].position.y - input[2].position.y)
//        + input[1].position.x * (input[2].position.y - input[0].position.y)
//        + input[2].position.x * (input[0].position.y - input[1].position.y));
//
//    // map fArea between [0,1]
//    float a = 3.12505290e-06;
//    float b = 0.00319999829;
//
//    fArea = (fArea - a) / (b - a);
//
//    for (int i = 0; i < 3; ++i)
//    {
//        // using position to represent color from this stage to save memory
//        output.position = input[i].position;
//        output.color = float4(fArea, 1.f, fArea, 1.f);
//        outStream.Append(output);
//    }
//
//}


//struct MSvert
//{
//	float4 pos : SV_POSITION;
//	float3 color : COLOR0;
//};
//
//[outputtopology("triangle")]
//[numthreads(12, 1, 1)]
//void MSMain(
//	in uint3 groupID : SV_GroupID,
//	in uint3 threadInGroup : SV_GroupThreadID,
//	out vertices MSvert verts[8],
//	out indices uint3 idx[12]) // Three indices per primitive
//{
//	// Although we're running on multiple threads, the threads share
//	// the same output arrays (e.g., of indices).
//	const uint numVertices = 8;
//	const uint numPrimitives = 12;
//	SetMeshOutputCounts(numVertices, numPrimitives);
//
//	const float4 allVertices[] = {
//		float4(-0.5f, -0.5f, -0.5f, 1.0f),
//		float4(-0.5f, -0.5f, 0.5f, 1.0f),
//		float4(-0.5f, 0.5f, -0.5f, 1.0f),
//		float4(-0.5f, 0.5f, 0.5f, 1.0f),
//		float4(0.5f, -0.5f, -0.5f, 1.0f),
//		float4(0.5f, -0.5f, 0.5f, 1.0f),
//		float4(0.5f, 0.5f, -0.5f, 1.0f),
//		float4(0.5f, 0.5f, 0.5f, 1.0f)
//	};
//
//	const float3 allColors[] = {
//		float3(0.0f, 0.0f, 0.0f),
//		float3(0.0f, 0.0f, 1.0f),
//		float3(0.0f, 1.0f, 0.0f),
//		float3(0.0f, 1.0f, 1.0f),
//		float3(1.0f, 0.0f, 0.0f),
//		float3(1.0f, 0.0f, 1.0f),
//		float3(1.0f, 1.0f, 0.0f),
//		float3(1.0f, 1.0f, 1.0f)
//	};
//
//	const uint3 allIndices[] = {
//		uint3(0, 2, 1),
//		uint3(1, 2, 3),
//		uint3(4, 5, 6),
//		uint3(5, 7, 6),
//		uint3(0, 1, 5),
//		uint3(0, 5, 4),
//		uint3(2, 6, 7),
//		uint3(2, 7, 3),
//		uint3(0, 4, 6),
//		uint3(0, 6, 2),
//		uint3(1, 3, 7),
//		uint3(1, 7, 5)
//	};
//
//	uint tid = threadInGroup.x;
//
//	if (tid < numVertices)
//	{
//		verts[tid].pos = allVertices[tid];
//		verts[tid].color = allColors[tid];
//	}
//
//	idx[tid] = allIndices[tid];
//}
