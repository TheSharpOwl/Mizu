#pragma once
#include "DX12LibPCH.h"

namespace Mizu
{
    class Resource;
    class ResourceStateTracker;
    class UploadBuffer;

    class CommandList
    {
    public:
        CommandList(D3D12_COMMAND_LIST_TYPE type);
        virtual ~CommandList();


        //D3D12_COMMAND_LIST_TYPE getCommandListType() const; TODO



        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> getGraphicsCommandList() // TODO
        {
            return m_commandList;
        }

        void setDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType, ID3D12DescriptorHeap* descriptorHeap) // TODO
        {
	        
	        
        }

        void copyResource(Resource& dstRes, const Resource& srcRes);

        void transitionBarrier(const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, bool flushBarriers = false);

        void flushResourceBarriers() // TODO
        {
	        
        }

        // the root parameter at rootParameterIndex should be set to D3D12_ROOT_PARAMETER_TYPE_CBV
        void setGraphicsDynamicConstantBuffer(uint32_t rootParameterIndex, size_t sizeInBytes, const void* bufferData);

    private:

        void trackResource(const Resource& res);

        void trackObject(Microsoft::WRL::ComPtr<ID3D12Object> object);

        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;

        std::unique_ptr<ResourceStateTracker> m_resourceStateTracker;

        std::unique_ptr<UploadBuffer> m_uploadBuffer;
    };
}

