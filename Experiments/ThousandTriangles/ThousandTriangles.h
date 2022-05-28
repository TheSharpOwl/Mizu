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

#pragma once

#include "DXSample.h"

using namespace DirectX;

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;

class ThousandTriangles : public DXSample
{
public:
    ThousandTriangles(UINT width, UINT height, float resizeAmount);

    virtual void OnInit();
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnDestroy();

private:
    static const UINT FrameCount = 2;

    struct Vertex
    {
        XMFLOAT3 position;
        XMFLOAT4 color;
    };

    Vertex m_firstTriangle[3] = {
    { {0.0f,  0.0f, 0.0f} ,{0.0f,  1.0f, 0.0f, 1.0f}},
    { {0.25f, 0.f, 0.0f}  ,{0.0f,  1.0f, 0.0f, 1.0f}},
    { {0.f,  -0.25f, 0.0f},{0.0f,  1.0f, 0.0f, 1.0f}}
    };

    float dataToUpload[1] = {10.0f};
    // TODO next time : rename the file of this class
    static const int T = 4800/200;
    float m_resizeAmount;

    Vertex m_triangles[T * 3];

    Vertex movePoint(const Vertex& point, float xDif, float yDif)
    {
        Vertex ans = point;
        ans.position.x += xDif;
        ans.position.y += yDif;
        return ans;
    }

    void generateTriangles();
    Vertex resizePoint(Vertex v);

    // Pipeline objects.
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12Device2> m_device;
    ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12PipelineState> m_pipelineState;
    ComPtr<ID3D12GraphicsCommandList6> m_commandList;
    UINT m_rtvDescriptorSize;

    // App resources.
    ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

    // data to upload
    std::vector<XMFLOAT3> m_meshShaderCoordsData;
    ComPtr<ID3D12Resource> m_structuredBuffer;
    // Synchronization objects.
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue;

    void LoadPipeline();
    void LoadAssets();
    void PopulateCommandList();
    void WaitForPreviousFrame();

    // mesh shader related stuff
    bool m_supportMeshShaders = true; // TODO change it to input dependent variable
	//Microsoft::WRL::ComPtr<ID3D12PipelineState> m_meshShaderPipelineState;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_meshShaderCoordsDescHeap;

    void createCoordsDescHeap();
};
