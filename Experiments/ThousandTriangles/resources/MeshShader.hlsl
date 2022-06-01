
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
[numthreads(64, 1, 1)]
void main(
	in uint3 groupID : SV_GroupID,
	in uint3 threadInGroup : SV_GroupThreadID,
	out vertices MSvert verts[255],
	out indices uint3 idx[85]) // Three indices per primitive
{
	const uint numVertices = 255;
	const uint numPrimitives = 85; // 192 / 3
	SetMeshOutputCounts(numVertices, numPrimitives);

	uint tid = threadInGroup.x;
	uint numOfTriangles = 4800 / 1;

	for(int i = 0; i < 4;i++)
	{
		if (i + tid * 4 >= 255)
			break;
		int k = subsetStart.start + (tid * 4) + i;
		if(k < numOfTriangles * 3)
		{
			verts[tid * 4 + i].pos = float4(coords[k].x, coords[k].y, coords[k].z, 1.f);
			verts[tid * 4 + i].color = float4(coords[k].w, 1.f, coords[k].w, 1.f);
		}
	}
	if(tid * 2 < 85)
	{
		idx[tid * 2] = uint3(tid * 3, tid * 3 + 1, tid * 3 + 2);
		idx[tid * 2 + 1] = uint3(tid * 3 + 3, tid * 3 + 4, tid * 3 + 5);
	}

	int x = 1000;
}
