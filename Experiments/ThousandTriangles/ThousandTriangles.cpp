//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "stdafx.h"
#include "ThousandTriangles.h"
#include <string>
#include <vector>
#include "Mizu/Utils.hpp"

#define MESH_SHADER

ThousandTriangles::ThousandTriangles(UINT width, UINT height, float resizeAmount) :
	DXSample(width, height, L"Thousand Triangles Experiment"),
	m_resizeAmount(resizeAmount),
	m_frameIndex(0),
	m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
	m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
	m_rtvDescriptorSize(0)
{
}

void ThousandTriangles::OnInit()
{
	LoadPipeline();
	LoadAssets();
}

// Load the rendering pipeline dependencies.
void ThousandTriangles::LoadPipeline()
{
	UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
	// Enable the debug layer (requires the Graphics Tools "optional feature").
	// NOTE: Enabling the debug layer after device creation will invalidate the active device.
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	ComPtr<IDXGIFactory4> factory;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

	if (m_useWarpDevice)
	{
		ComPtr<IDXGIAdapter> warpAdapter;
		ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			warpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_device)
		));
	}
	else
	{
		ComPtr<IDXGIAdapter1> hardwareAdapter;
		GetHardwareAdapter(factory.Get(), &hardwareAdapter);

		ThrowIfFailed(D3D12CreateDevice(
			hardwareAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_device)
		));
	}

	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),							// Swap chain needs the queue so that it can force a flush on it.
		Win32Application::GetHwnd(),
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	// This experiment doesn't support fullscreen transitions.
	ThrowIfFailed(factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

	ThrowIfFailed(swapChain.As(&m_swapChain));
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

		m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	// Create frame resources.
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		// Create an RTV for each frame.
		for (UINT n = 0; n < FrameCount; n++)
		{
			ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
			m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_rtvDescriptorSize);
		}
	}

	ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
}

// Load the sample assets.
void ThousandTriangles::LoadAssets()
{
	{
#ifdef MESH_SHADER
		// define the range
		D3D12_DESCRIPTOR_RANGE srvRange;
		ZeroMemory(&srvRange, sizeof(srvRange));
		srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		srvRange.NumDescriptors = 1; // only upload one buffer
		srvRange.BaseShaderRegister = 0; // starting from the first register t0 (t is made for srv of course)
		srvRange.RegisterSpace = 0; // this allows to use same register name if a different space is used (doesn't matter in this context)
		srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// define the root parameter (the structured buffer)
		D3D12_ROOT_PARAMETER srvRootParameter;
		ZeroMemory(&srvRootParameter, sizeof(srvRootParameter));
		srvRootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		srvRootParameter.DescriptorTable = { 1, &srvRange };// one range
		srvRootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_MESH; // only visible to mesh shader

		// define the constant buffer (subset integer)
		D3D12_ROOT_PARAMETER subsetRootParameter;
		ZeroMemory(&subsetRootParameter, sizeof(subsetRootParameter));
		subsetRootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		subsetRootParameter.Constants = { 0,0,1 };// register b0, first register space and 1 value
		subsetRootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_MESH;

		std::vector<D3D12_ROOT_PARAMETER> rootParameters{ srvRootParameter, subsetRootParameter};

		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags{
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS
		};

		D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		ZeroMemory(&rootSignatureDesc, sizeof(rootSignatureDesc));
		rootSignatureDesc.NumParameters = static_cast<UINT>(rootParameters.size());
		rootSignatureDesc.pParameters = rootParameters.data();
		rootSignatureDesc.NumStaticSamplers = 0; // samplers can be stored in root signature separately and consume no space
		rootSignatureDesc.pStaticSamplers = nullptr; // no texturing
		rootSignatureDesc.Flags = rootSignatureFlags;

#else
		// Create an empty root signature.
		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
#endif

		// Left for reference, defining a root signature using helper functions
		/*
		struct
		{
			byte* data;
			uint32_t size;
		} meshShaderData;

		ReadDataFromFile(L"MeshShader.cso", &meshShaderData.data, &meshShaderData.size);
		ThrowIfFailed(m_device->CreateRootSignature(0, meshShaderData.data, meshShaderData.size, IID_PPV_ARGS(&m_rootSignature)));
		*/
	
		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
		ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
	}

	// Create the pipeline state, which includes compiling and loading shaders.
	{

		ComPtr<ID3DBlob> pixelShader;
		ComPtr<ID3DBlob> geomertyShader;
#ifdef MESH_SHADER
		ComPtr<ID3DBlob> meshShader;
#else
		ComPtr<ID3DBlob> vertexShader;
#endif


#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif


		ThrowIfFailed(D3DReadFileToBlob(L"ThousandTrianglesPixelShader.cso", &pixelShader));

#ifdef MESH_SHADER
		ThrowIfFailed(D3DReadFileToBlob(L"MeshShader.cso", &meshShader));
#else
		ThrowIfFailed(D3DReadFileToBlob(L"ThousandTrianglesVertexShader.cso", &vertexShader));
#endif


#ifdef MESH_SHADER
		// no need to to add any input elements for the mesh shader
#else
		// Define the vertex input layout.
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
#endif

#ifdef MESH_SHADER
		// using the new way to define the pipeline state (define only needed fields and the rest will be default)
		struct PSO_STREAM
		{
			CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
			CD3DX12_PIPELINE_STATE_STREAM_MS MS;
			CD3DX12_PIPELINE_STATE_STREAM_PS PS;
			CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER RasterizerState;
			CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC BlendState;
			CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL DepthStencilState;
			CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_MASK SampleMask;
			CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopology;
			CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RenderTargetFormats;
			CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DepthStencilFormat;
			CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC SampleDesc;
		} stateStream;

		stateStream.pRootSignature = m_rootSignature.Get();
		stateStream.MS = CD3DX12_SHADER_BYTECODE(meshShader.Get());
		stateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		stateStream.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		stateStream.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

		// disable depth stencil buffer 
		CD3DX12_DEPTH_STENCIL_DESC& depthStencilState = stateStream.DepthStencilState;
		depthStencilState.DepthEnable = FALSE;
		depthStencilState.StencilEnable = FALSE;

		stateStream.SampleMask = UINT_MAX;
		stateStream.PrimitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		D3D12_RT_FORMAT_ARRAY rtFormatArray{};
		rtFormatArray.NumRenderTargets = 1;
		rtFormatArray.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		stateStream.RenderTargetFormats = rtFormatArray;

		DXGI_SAMPLE_DESC sampleDesc{};
		sampleDesc.Count = 1;
		stateStream.SampleDesc = sampleDesc;

		D3D12_PIPELINE_STATE_STREAM_DESC StreamDesc;
		StreamDesc.pPipelineStateSubobjectStream = &stateStream;
		StreamDesc.SizeInBytes = sizeof(stateStream);
		ThrowIfFailed(m_device->CreatePipelineState(&StreamDesc, IID_PPV_ARGS(&m_pipelineState)));
#else
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;
		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
#endif
	}

	// Create the command list.
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));

	// Command lists are created in the recording state, but nothing to record yet and main loop expects it closed so close for now
	ThrowIfFailed(m_commandList->Close());

	// generate the triangles (only once at the start of the code)
	generateTriangles();
#ifdef MESH_SHADER

	using coordsType = decltype(ThousandTriangles::m_meshShaderCoordsData)::value_type;
	// create a default buffer


	//m_meshShaderCoordsData.push_back({ -0.5f, 0.0f, 0.0f, 1.0f });
	//m_meshShaderCoordsData.push_back({ 0.25f, 0.0f, 0.0f, 1.0f });
	//m_meshShaderCoordsData.push_back({ 0.0f, -0.25f, 0.0f, 1.0f });

	m_structuredBuffer = Mizu::createStructuredBuffer(m_device.Get(), m_meshShaderCoordsData, L"coords");

	// create descriptor heap
	createCoordsDescHeap();

	Mizu::createSrv<coordsType>(m_device.Get(), m_meshShaderCoordsDescHeap.Get(), 0, m_structuredBuffer.Get(), m_meshShaderCoordsData.size());
#else
	// Create the vertex buffer.
	{

		const UINT vertexBufferSize = sizeof(m_triangles);

		// Note: using upload heaps to transfer static data like vert buffers is not 
		// recommended. Every time the GPU needs it, the upload heap will be marshalled 
		// over. Please read up on Default Heap usage. An upload heap is used here for 
		// code simplicity and because there are very few verts to actually transfer.
		// TODO optimize this !!!!!!!!
		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_vertexBuffer)));

		// Copy the triangle data to the vertex buffer.
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
		ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, m_triangles, sizeof(m_triangles));
		m_vertexBuffer->Unmap(0, nullptr);

		// Initialize the vertex buffer view.
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.StrideInBytes = sizeof(Vertex);
		m_vertexBufferView.SizeInBytes = vertexBufferSize;
	}
#endif
	// Create synchronization objects and wait until assets have been uploaded to the GPU.
	{
		ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
		m_fenceValue = 1;

		// Create an event handle to use for frame synchronization.
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}

		// Wait for the command list to execute; we are reusing the same command 
		// list in our main loop but for now, we just want to wait for setup to 
		// complete before continuing.
		WaitForPreviousFrame();
	}
}

// Update frame-based values.
void ThousandTriangles::OnUpdate()
{
}

// Render the scene.
void ThousandTriangles::OnRender()
{
	// Record all the commands we need to render the scene into the command list.
	PopulateCommandList();

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Present the frame.
	ThrowIfFailed(m_swapChain->Present(0, 0));

	WaitForPreviousFrame();
}

void ThousandTriangles::OnDestroy()
{
	// Ensure that the GPU is no longer referencing resources that are about to be
	// cleaned up by the destructor.
	WaitForPreviousFrame();

	CloseHandle(m_fenceEvent);
}

void ThousandTriangles::generateTriangles()
{
	Vertex firstTriangle[3] = {
{ {0.0f,  0.0f, 0.0f} ,{0.0f,  1.0f, 0.0f, 1.0f}},
{ {0.25f, 0.f, 0.0f}  ,{0.0f,  1.0f, 0.0f, 1.0f}},
{ {0.f,  -0.25f, 0.0f},{0.0f,  1.0f, 0.0f, 1.0f}}
	};

	double fArea = -1.f;

	auto onScreen = [](Vertex v)
	{
		return v.position.x >= -1.f && v.position.x <= 1.f && v.position.y >= -1.f && v.position.y <= 1.f;
	};

	auto movePoint = [](const Vertex& point, float xDif, float yDif)
	{
		Vertex ans = point;
		ans.position.x += xDif;
		ans.position.y += yDif;
		return ans;
	};

	auto resizePoint = [this](Vertex v)
	{
		Vertex ans = v;
		ans.position.x *= m_resizeAmount;
		ans.position.y *= m_resizeAmount;
		ans.position.z *= m_resizeAmount;
		return ans;
	};

	for (int i = 0; i < T * 3; i += 3)
	{
		while (true)
		{
			// get a random point on screen
			float x = Mizu::mapToScreen(rand() % 1280, 0, 1279);
			float y = Mizu::mapToScreen(rand() % 720, 0, 719);
			// try it
			m_triangles[i] = movePoint(resizePoint(firstTriangle[0]), x, y);
			m_triangles[i + 1] = movePoint(resizePoint(firstTriangle[1]), x, y);
			m_triangles[i + 2] = movePoint(resizePoint(firstTriangle[2]), x, y);
			// break if it is according to the conditions
			if (onScreen(m_triangles[i]) && onScreen(m_triangles[i + 1]) && onScreen(m_triangles[i + 2]))
				break;
		}

		if (i < 3) // only calculate area once
		{
			fArea = 0.5f * abs(m_triangles[0].position.x * (m_triangles[1].position.y - m_triangles[2].position.y)
				+ m_triangles[1].position.x * (m_triangles[2].position.y - m_triangles[0].position.y)
				+ m_triangles[2].position.x * (m_triangles[0].position.y - m_triangles[1].position.y));

			assert(fArea > 0.f);

			double a = 3.1250528991222382e-06;// min size of the triangle
			double b = 0.00025594513863325119; // max size of the triangle (for rgb time)
			//double b = 0.031250000000000000; // for 1.f

			// map fArea between [0,1] 
			fArea = (fArea - a);
			fArea /= (b - a);
			assert(fArea >= 0.0);
		}
		// set the green color level depending on the size
		for (int j = 0; j < 3; j++)
		{
			assert(fArea >= 0.0);
			m_triangles[i + j].color.x = m_triangles[i + j].color.z = static_cast<float>(fArea); // bigger = more white
#ifdef MESH_SHADER
			// We just care about all needed information arriving to the mesh shader (and there it will be used correctly)
			m_meshShaderCoordsData.push_back({ m_triangles[i + j].position.x, m_triangles[i + j].position.y, m_triangles[i + j].position.z, m_triangles[i + j].color.x });
#endif
		}

		// TODO upload data to the mesh shader from here (use a float4 where the first 3 are position and the 4th is the color to not write upload of another buffer)
	}
}

void ThousandTriangles::PopulateCommandList()
{
	// Command list allocators can only be reset when the associated 
	// command lists have finished execution on the GPU; apps should use 
	// fences to determine GPU execution progress.
	ThrowIfFailed(m_commandAllocator->Reset());

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
#ifdef MESH_SHADER
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()));
#else
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()));
#endif

	// Set necessary state.
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);
	// Indicate that the back buffer will be used as a render target.
	ID3D12Resource* currBuffer{ m_renderTargets[m_frameIndex].Get()};

	D3D12_RESOURCE_BARRIER barrierDesc;
	ZeroMemory(&barrierDesc, sizeof(barrierDesc));
	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Transition.pResource = currBuffer;
	barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	m_commandList->ResourceBarrier(1, &barrierDesc);

	static UINT descriptorSize{ m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV) };
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	rtvHandle.ptr += m_frameIndex * descriptorSize;

	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	// no need to clear depth because no depth is defined
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
#ifdef MESH_SHADER
	ID3D12DescriptorHeap* ppHeaps[] = { m_meshShaderCoordsDescHeap.Get() };
	m_commandList->SetDescriptorHeaps(1, ppHeaps);
	// TODO understand these lines
	D3D12_GPU_DESCRIPTOR_HANDLE d = m_meshShaderCoordsDescHeap->GetGPUDescriptorHandleForHeapStart();
	d.ptr += 0;
	m_commandList->SetGraphicsRootDescriptorTable(0, d);

	for(uint32_t i = 0;i < T * 3;i += 75)
	{
		const std::vector<uint32_t> rootConstants{i};
		// root parameter index is 1 (the letter b in b0 doesn't matter here because when setting the root parameters it was the second one in the vector so it's index overall not inside b)
		m_commandList->SetGraphicsRoot32BitConstant(1, i, 0);
		m_commandList->DispatchMesh(75, 1, 1);
	}
#else
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_commandList->DrawInstanced(T * 3, T, 0, 0);
#endif
	// Indicate that the back buffer will now be used to present.
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(m_commandList->Close());
}

void ThousandTriangles::WaitForPreviousFrame()
{
	// Signal and increment the fence value.
	const UINT64 fence = m_fenceValue;
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence));
	m_fenceValue++;

	// Wait until the previous frame is finished.
	if (m_fence->GetCompletedValue() < fence)
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void ThousandTriangles::createCoordsDescHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	ZeroMemory(&heapDesc, sizeof(heapDesc));
	heapDesc.NumDescriptors = 1; // only one structured buffer for the coords one
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.NodeMask = 0;

	ThrowIfFailed(m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_meshShaderCoordsDescHeap)));
}
