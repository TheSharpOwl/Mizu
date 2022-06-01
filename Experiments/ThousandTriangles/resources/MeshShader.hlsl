
struct MSvert
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

struct SubsetIndicator // because int cannot be used directly and it should be a struct
{
	uint start;
};

StructuredBuffer<float4> coords : register(t0);

ConstantBuffer<SubsetIndicator> subsetStart : register(b0);


[outputtopology("triangle")]
[numthreads(75, 1, 1)]
void main(
	in uint3 groupID : SV_GroupID,
	in uint3 threadInGroup : SV_GroupThreadID,
	out vertices MSvert verts[225],
	out indices uint3 idx[75]) // Three indices per primitive
{
	const uint numVertices = 225;
	const uint numPrimitives = 75; // 225 / 3
	SetMeshOutputCounts(numVertices, numPrimitives);

	uint tid = threadInGroup.x;

	// each thread 1 triangle
	for(int i = 0; i < 3; i++)
	{
		int j = tid * 3 + i;
		int k = subsetStart.start + j;
		verts[j].pos = float4(coords[k].x, coords[k].y, coords[k].z, 1.f);
		verts[j].color = float4(coords[k].w, 1.f, coords[k].w, 1.f);
	}

	idx[tid] = uint3(tid * 3, tid * 3 + 1, tid * 3 + 2);
}
