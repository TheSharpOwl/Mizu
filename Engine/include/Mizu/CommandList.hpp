#pragma once
#include "DX12LibPCH.h"

namespace Mizu
{
    class CommandList
    {
    public:
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> getGraphicsCommandList()
        {
            return m_commandList;
        }

        void setDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType, ID3D12DescriptorHeap* descriptorHeap)
        {
	        
	        
        }

    private:
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
    };
}

