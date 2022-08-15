#pragma once
#include "DX12LibPCH.h"

namespace Mizu
{
    class Resource;
    class ResourceStateTracker;

    class CommandList
    {
    public:
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> getGraphicsCommandList() // TODO
        {
            return m_commandList;
        }

        void setDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType, ID3D12DescriptorHeap* descriptorHeap) // TODO
        {
	        
	        
        }

        void TransitionBarrier(const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource, bool flushBarriers);

        void flushResourceBarriers() // TODO
        {
	        
        }

    private:
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;

        std::unique_ptr<ResourceStateTracker> m_resourceStateTracker;
    };
}

