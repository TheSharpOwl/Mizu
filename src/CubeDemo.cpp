#include "..\inc\CubeDemo.hpp"
#include <Application.hpp>


using namespace Mizu;
using namespace DirectX;

struct VertexPosColor
{
	XMFLOAT3 Position;
	XMFLOAT3 Color;
};

static VertexPosColor Vertices[8] = {
	{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) }, // 0
	{ XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) }, // 1
	{ XMFLOAT3(1.0f,  1.0f, -1.0f),  XMFLOAT3(1.0f, 1.0f, 0.0f) }, // 2
	{ XMFLOAT3(1.0f, -1.0f, -1.0f),  XMFLOAT3(1.0f, 0.0f, 0.0f) }, // 3
	{ XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) }, // 4
	{ XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 1.0f) }, // 5
	{ XMFLOAT3(1.0f,  1.0f,  1.0f),  XMFLOAT3(1.0f, 1.0f, 1.0f) }, // 6
	{ XMFLOAT3(1.0f, -1.0f,  1.0f),  XMFLOAT3(1.0f, 0.0f, 1.0f) }  // 7
};


static WORD Indecies[36] =
{
	0, 1, 2, 0, 2, 3,
	4, 6, 5, 4, 7, 6,
	4, 5, 1, 4, 1, 0,
	3, 2, 6, 3, 6, 7,
	1, 5, 6, 1, 6, 2,
	4, 0, 3, 4, 3, 7
};

CubeDemo::CubeDemo(int width, int height, bool vsync) :
	m_width(width), m_height(height), m_vsync(vsync)
{

}

bool CubeDemo::create(HINSTANCE hInst)
{
	if (DirectX::XMVerifyCPUSupport())
	{
		MessageBoxA(NULL, "Failed to verify DirectX math support", "Error", MB_OK | MB_ICONERROR);
		return false;
	}

	// might need to put a pointer to this demo in the window later (for controls of mouse and keyboard for example)
	Application::Create(hInst);

	return true;
}

bool CubeDemo::LoadContent()
{
	auto device = Application::Get().GetDevice();
	auto commandQueue = Application::Get().GetCommandQueue();
	// TODO next time : make the command lists 3 because now we need copy one and the one we have already is only direct 
	auto commandList = Application::Get().GetCommandList();
	return false;
}

void Mizu::CubeDemo::UpdateBufferResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
	ID3D12Resource** ppDestinationRes,
	ID3D12Resource** ppIntermediateRes,
	size_t numElements,
	size_t elementSize,
	const void* bufferData,
	D3D12_RESOURCE_FLAGS flags)
{

	auto device = Application::Get().GetDevice();
	size_t bufferSize = numElements * elementSize;

	// Create the committed resource for the GPU part in a default heap
	ThrowIfFailed(
		device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(ppDestinationRes)
		)
	);

	if (bufferData)
	{
		// create committed resource to upload from it
		ThrowIfFailed(
			device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(ppIntermediateRes)
			)
		);

		D3D12_SUBRESOURCE_DATA subresourceData = {};
		subresourceData.pData = bufferData;
		// TODO check this two Row and Slice what they are and why do it this way?
		subresourceData.RowPitch = subresourceData.SlicePitch = bufferSize;

		UpdateSubresources(commandList.Get(), *ppDestinationRes, *ppIntermediateRes, 0, 0, 1, &subresourceData);
	}
}

