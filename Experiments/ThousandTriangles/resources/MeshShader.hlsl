
#define ROOT_SIG "SRV(t0)"

struct MSvert
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

struct Vertex
{
	float3 pos;
	float4 color;
};

StructuredBuffer<Vertex> Globals : register(t0);

[RootSignature(ROOT_SIG)]
[outputtopology("triangle")]
[numthreads(3, 1, 1)]
void main(
	in uint3 groupID : SV_GroupID,
	in uint3 threadInGroup : SV_GroupThreadID,
	out vertices MSvert verts[3],
	out indices uint3 idx[1]) // Three indices per primitive
{
	const uint numVertices = 3;
	const uint numPrimitives = 1;
	SetMeshOutputCounts(numVertices, numPrimitives);

	const float4 allVertices[] = {
		float4(0.0f, 0.0f, 0.0f, 1.0f),
		float4(0.25f, 0.0f, 0.0f, 1.0f),
		float4(0.0f, -0.25f, 0.0f, 1.0f),
	};

	const float4 allColors[] = {
		float4(0.0f,  1.0f, 0.0f, 1.0f),
		float4(0.0f,  1.0f, 0.0f, 1.0f),
		float4(0.0f,  1.0f, 0.0f, 1.0f),
	};

	uint tid = threadInGroup.x;

	const uint3 allIndices[] = {
	uint3(0, 1, 2),
	};

	if (tid < numVertices)
	{
		//verts[tid].pos = float4(Globals[tid].pos, 0.f);
		//verts[tid].color = Globals[tid].color;
		//idx[tid] = tid;

		verts[tid].pos = allVertices[tid];
		verts[tid].color = allColors[tid];
		idx[tid] = allIndices[tid];
	}
}
