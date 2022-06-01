
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
[numthreads(96, 1, 1)]
void main(
	in uint3 groupID : SV_GroupID,
	in uint3 threadInGroup : SV_GroupThreadID,
	out vertices MSvert verts[96],
	out indices uint3 idx[32]) // Three indices per primitive
{
	const uint numVertices = 96;
	const uint numPrimitives = 32; // 96 / 3
	SetMeshOutputCounts(numVertices, numPrimitives);

	uint tid = threadInGroup.x;
	uint numOfTriangles = 4800 / 1;

	int k = subsetStart.start + tid;

	if (k < numOfTriangles * 3)
	{
		verts[tid].pos = float4(coords[k].x, coords[k].y, coords[k].z, 1.f);
		verts[tid].color = float4(coords[k].w, 1.f, coords[k].w, 1.f);
	}

	if(tid < 32 && subsetStart.start + tid * 3 < numOfTriangles * 3)
	{
		idx[tid] = uint3(tid * 3, tid * 3 + 1, tid * 3 + 2);
	}
}
