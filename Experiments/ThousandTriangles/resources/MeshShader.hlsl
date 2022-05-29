
struct MSvert
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};


StructuredBuffer<float4> coords : register(t0);

[outputtopology("triangle")]
[numthreads(128, 1, 1)]
void main(
	in uint3 groupID : SV_GroupID,
	in uint3 threadInGroup : SV_GroupThreadID,
	out vertices MSvert verts[255],
	out indices uint3 idx[85]) // Three indices per primitive
{
	const uint numVertices = 255;
	const uint numPrimitives = 255 / 3;
	SetMeshOutputCounts(numVertices, numPrimitives);

	const float4 allColors[] = {
		float4(0.0f,  1.0f, 0.0f, 1.0f),
		float4(0.0f,  1.0f, 0.0f, 1.0f),
		float4(0.0f,  1.0f, 0.0f, 1.0f),
	};

	uint tid = threadInGroup.x;

	//const uint3 allIndices[] = {
	//uint3(0, 1, 2),
	//};

	if (tid < numVertices)
	{
		verts[tid].pos = float4(coords[tid].x, coords[tid].y, coords[tid].z, 1.f);
		verts[tid].color = float4(coords[tid].w, 1.f, coords[tid].w, 1.f);

		if (tid < numPrimitives)
			idx[tid] = uint3(tid * 3, tid * 3 + 1, tid * 3 + 2);
	}
}
