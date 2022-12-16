#include "CubeDemo.hpp"

#include <windows.h>
#include "directxmath.h"
#include <string>

#include "Mizu/Application.hpp"
#include "Mizu/CommandQueue.hpp"
#include "Mizu/EventArgs.hpp"

using namespace DirectX;
using Mizu::CubeDemo;

template<typename T>
using cp = Microsoft::WRL::ComPtr<T>;

struct VertexPosColor
{
	XMFLOAT3 Position;
	XMFLOAT3 Color;
};

static VertexPosColor Vertices[8] = {
	{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) }, // 0
	{ XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) }, // 1
	{ XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) }, // 2
	{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) }, // 3
	{ XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) }, // 4
	{ XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 1.0f) }, // 5
	{ XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) }, // 6
	{ XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT3(1.0f, 0.0f, 1.0f) }  // 7
};

static WORD Indicies[36] =
{
	0, 1, 2, 0, 2, 3,
	4, 6, 5, 4, 7, 6,
	4, 5, 1, 4, 1, 0,
	3, 2, 6, 3, 6, 7,
	1, 5, 6, 1, 6, 2,
	4, 0, 3, 4, 3, 7
};

CubeDemo::CubeDemo(int width, int height, bool vsync) :
	Game(L"CubeDemo", width, height, vsync)
	, m_viewport(CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)))
	, m_scissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX))
	, m_fov(45.0)
	, m_contentLoaded(false)
{

}

bool CubeDemo::LoadContent()
{
	auto device = Application::Get().GetDevice();
	auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
	auto commandList = commandQueue->GetCommandList();

	// Uploading vertex buffer data
	cp<ID3D12Resource> intermediateVertexBuffer;
	UpdateBufferResource(commandList, &m_vertexBuffer, &intermediateVertexBuffer, _countof(Vertices), sizeof(VertexPosColor), Vertices);

	// Create the vertex buffer view
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = sizeof(Vertices);
	m_vertexBufferView.StrideInBytes = sizeof(VertexPosColor);

	// Uploading index buffer data
	cp<ID3D12Resource> intermediateIndexBuffer;
	UpdateBufferResource(commandList, &m_indexBuffer, &intermediateIndexBuffer, _countof(Indicies), sizeof(WORD), Indicies);

	//Creating the index buffer view
	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	m_indexBufferView.SizeInBytes = sizeof(Indicies);

	// creation of the desc heap for the depth stencil view
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ThrowIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));


	// Loading shaders
	cp<ID3DBlob> vertexShaderBlob;
	ThrowIfFailed(D3DReadFileToBlob(L"VertexShader.cso", &vertexShaderBlob));

	cp<ID3DBlob> pixelShaderBlob;
	ThrowIfFailed(D3DReadFileToBlob(L"PixelShader.cso", &pixelShaderBlob));

	// Create the vertex input layout
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	// Creating a root signature
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
	if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	// Allow input layout and deny unnecessary access to certain pipeline stages ( minor optimization on some hardware)
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	CD3DX12_ROOT_PARAMETER1 rootParamters[1];
	// first parameter was sizeof(XMatrix)/4 (same number)
	rootParamters[0].InitAsConstants(16u, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootsSignatureDescription;
	rootsSignatureDescription.Init_1_1(_countof(rootParamters), rootParamters, 0, nullptr, rootSignatureFlags);

	// serialization of the root signature (better to convert it to binary before runtime ... todo)
	cp<ID3DBlob> rootSignatureBlob;
	cp<ID3DBlob> errorBlob;

	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootsSignatureDescription, featureData.HighestVersion, &rootSignatureBlob, &errorBlob));

	// create the root signature
	ThrowIfFailed(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)));

	struct PipelineStateStream
	{
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
		CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
		CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
		CD3DX12_PIPELINE_STATE_STREAM_VS VS;
		CD3DX12_PIPELINE_STATE_STREAM_PS PS;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
	} pipelineStateStream;

	D3D12_RT_FORMAT_ARRAY rtvFormats = {};
	rtvFormats.NumRenderTargets = 1;
	rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	pipelineStateStream.pRootSignature = m_RootSignature.Get();
	pipelineStateStream.InputLayout = { inputLayout, _countof(inputLayout) };
	pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
	pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
	pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineStateStream.RTVFormats = rtvFormats;

	D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
		sizeof(PipelineStateStream), &pipelineStateStream
	};

	ThrowIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&m_pipelineState)));

	auto fenceValue = commandQueue->executeCommandList(commandList);
	commandQueue->waitForFenceValue(fenceValue);

	m_contentLoaded = true;

	// Resize/create the depth buffer
	ResizeDepthBuffer(m_width, m_height);


	return true;
}

void CubeDemo::OnResize(ResizeEventArgs& e)
{
	if (m_width == e.width && m_height == e.height)
		return;

	m_width = e.width;
	m_height = e.height;
	m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(e.width), static_cast<float>(e.height));

	ResizeDepthBuffer(e.width, e.height);
}

void CubeDemo::OnUpdate(UpdateEventArgs& e)
{
	float angle = static_cast<float>(e.totalTime * 90.0);
	const XMVECTOR rotationAxis = XMVectorSet(0, 1, 1, 0);
	m_modelMatrix = XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(angle));

	// Update the view matrix
	const XMVECTOR eyePos = XMVectorSet(0, 0, -10, 1);
	const XMVECTOR focusPoint = XMVectorSet(0, 0, 0, 1);
	const XMVECTOR upDirection = XMVectorSet(0, 1, 0, 0);
	m_viewMatrix = XMMatrixLookAtLH(eyePos, focusPoint, upDirection);

	// Update the projection matrix
	float aspectRatio = m_width / static_cast<float>(m_height);
	m_projectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(m_fov), aspectRatio, 0.1f, 100.0f);
}

void CubeDemo::OnRender(RenderEventArgs& e)
{
	auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	auto commandList = commandQueue->GetCommandList();
	// no need to reset the command list (check CommandQueue::GetCommandList function above)

	UINT currentBackBufferIndex = m_window->GetCurrentBackBufferIndex();
	auto backBuffer = m_window->getCurrentBackBuffer();
	auto rtv = m_window->GetRTV();
	auto dsv = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();

	// Clearing render targets
	{
		TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		FLOAT clearColor[] = { 0.7, 0.6f, 0.9f, 1.0f };

		ClearRTV(commandList, rtv, clearColor);
		ClearDepth(commandList, dsv);
	}

	commandList->SetPipelineState(m_pipelineState.Get());
	commandList->SetGraphicsRootSignature(m_RootSignature.Get());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	commandList->IASetIndexBuffer(&m_indexBufferView);

	commandList->RSSetViewports(1, &m_viewport);
	commandList->RSSetScissorRects(1, &m_scissorRect);

	commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

	// update MVP matrix
	XMMATRIX mvpMatrix = XMMatrixMultiply(m_modelMatrix, m_viewMatrix);
	mvpMatrix = XMMatrixMultiply(mvpMatrix, m_projectionMatrix);
	commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvpMatrix, 0);

	commandList->DrawIndexedInstanced(_countof(Indicies), 1, 0, 0, 0);

	// presenting
	{
		TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		m_fenceValues[currentBackBufferIndex] = commandQueue->executeCommandList(commandList);

		UINT presentFlags = Application::Get().IsTearingSupported() && !m_vsync ? DXGI_PRESENT_ALLOW_TEARING : 0;
		currentBackBufferIndex = m_window->Present((m_vsync ? 1 : 0), presentFlags);
		commandQueue->waitForFenceValue(m_fenceValues[currentBackBufferIndex]);
	}
}

void CubeDemo::UpdateBufferResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
	ID3D12Resource** ppDestinationRes,
	ID3D12Resource** ppIntermediateRes,
	size_t numElements,
	size_t elementSize,
	const void* bufferData,
	D3D12_RESOURCE_FLAGS flags) // flags = D3D12_RESOURCE_FLAG_NONE
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

void CubeDemo::ResizeDepthBuffer(int width, int height)
{
	if (m_contentLoaded) // will be true only after the descriptor heap is created
	{
		// Flush all gpu commands (because they might be referencing the depth buffer)
		Application::Get().Flush();

		width = std::max(width, 1);
		height = std::max(height, 1);

		auto device = Application::Get().GetDevice();

		D3D12_CLEAR_VALUE optimizedClearValue = {};
		optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		// 1 is for depth (far = 1), and 0 is for stencil
		optimizedClearValue.DepthStencil = { 1.0f, 0 };


		ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
			D3D12_RESOURCE_STATE_DEPTH_WRITE, &optimizedClearValue, IID_PPV_ARGS(&m_depthBuffer)));

		// Updating the depth stencil view
		D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
		dsv.Format = DXGI_FORMAT_D32_FLOAT;
		dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsv.Texture2D.MipSlice = 0;
		dsv.Flags = D3D12_DSV_FLAG_NONE;

		device->CreateDepthStencilView(m_depthBuffer.Get(), &dsv, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	}
}

void CubeDemo::TransitionResource(cp<ID3D12GraphicsCommandList2> commandList, cp<ID3D12Resource> resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), beforeState, afterState);
	commandList->ResourceBarrier(1, &barrier);
}


void CubeDemo::ClearRTV(cp<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor)
{
	commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
}

void CubeDemo::ClearDepth(cp<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth) // depth = 1.0f default
{
	commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
}

void CubeDemo::UnloadContent()
{
	m_contentLoaded = false;
}