
struct MSvert
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};


StructuredBuffer<float4> coords : register(t0);

[outputtopology("triangle")]
[numthreads(75, 1, 1)]
void main(
	in uint3 groupID : SV_GroupID,
	in uint3 threadInGroup : SV_GroupThreadID,
	out vertices MSvert verts[225],
	out indices uint3 idx[75]) // Three indices per primitive
{
	const uint numVertices = 225;
	const uint numPrimitives = 225 / 3; // 75
	SetMeshOutputCounts(numVertices, numPrimitives);

	const float4 allColors[] = {
		float4(0.0f,  1.0f, 0.0f, 1.0f),
		float4(0.0f,  1.0f, 0.0f, 1.0f),
		float4(0.0f,  1.0f, 0.0f, 1.0f),
	};

	uint tid = threadInGroup.x;

	// each thread 1 triangle
	for(int i = 0; i < 3; i++)
	{
		int j = tid * 3 + i;
		verts[j].pos = float4(coords[j].x, coords[j].y, coords[j].z, 1.f);
		verts[j].color = float4(coords[j].w, 1.f, coords[j].w, 1.f);
	}

	idx[tid] = uint3(tid * 3, tid * 3 + 1, tid * 3 + 2);
}
