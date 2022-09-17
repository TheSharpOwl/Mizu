#pragma once
#include "DX12LibPCH.h"
#include "DynamicDescriptorHeap.hpp"

namespace Mizu
{
    class Resource;
    class ResourceStateTracker;
    class UploadBuffer;
    class DynamicDescriptorHeap;

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

        /**
         * \brief 
         * \param rootParameterIndex to assign the SRV to
         * \param descriptorOffset
         * \param resource 
         * \param stateAfter 
         * \param firstSubresource to transit to the requested state 
         * \param numSubresources number of resources to transit
         * \param srv to use for the resource in the shader. by default nullptr which uses the default srv for the resource
         */
        void setShaderResourceView(
            uint32_t rootParameterIndex,
            uint32_t descriptorOffset,
            const Resource& resource,
            D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			UINT firstSubresource = 0,
            UINT numSubresources = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
            const D3D12_SHADER_RESOURCE_VIEW_DESC* srv = nullptr
        );

        void setUnorderedAccessView(uint32_t rootParameterIndex,
            uint32_t descriptorOffset,
            const Resource& resource,
            D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            UINT firstSubresource = 0,
            UINT numSubresources = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
            const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav= nullptr);

        /**
         * \brief Draw geomerty to the currently bound render target
         * \param vertexCount 
         * \param instanceCount 
         * \param startVertex 
         * \param startInstance 
         */
        void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t startVertex, uint32_t startInstance);

    protected:


        // TODO next time add comments to this class

        void trackResource(const Resource& res);

        void trackObject(Microsoft::WRL::ComPtr<ID3D12Object> object);


        std::unique_ptr<DynamicDescriptorHeap> m_dynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

        std::vector<Microsoft::WRL::ComPtr<ID3D12Object>> m_trackedObjects;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
        std::unique_ptr<ResourceStateTracker> m_resourceStateTracker;
        std::unique_ptr<UploadBuffer> m_uploadBuffer;
    };
}

