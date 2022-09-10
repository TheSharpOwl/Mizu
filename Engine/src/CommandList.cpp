#include "CommandList.hpp"

#include "DynamicDescriptorHeap.hpp"
#include "Resource.hpp"
#include "ResourceStateTracker.hpp"
#include "UploadBuffer.hpp"


namespace Mizu
{
	void CommandList::transitionBarrier(const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource, bool flushBarriers)
	{
		auto pResource = resource.getResource();
		if (pResource)
		{
			// we don't care about the before state because the resource state tracker will fix it for us
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(pResource.Get(), D3D12_RESOURCE_STATE_COMMON, stateAfter, subResource);
			m_resourceStateTracker->resourceBarrier(barrier);
		}

		if (flushBarriers)
		{
			flushResourceBarriers();
		}
	}

	void CommandList::copyResource(Resource& dstRes, const Resource& srcRes)
	{
		transitionBarrier(dstRes, D3D12_RESOURCE_STATE_COPY_DEST);
		transitionBarrier(srcRes, D3D12_RESOURCE_STATE_COPY_SOURCE);

		// to commit the resources
		flushResourceBarriers();

		m_commandList->CopyResource(dstRes.getResource().Get(), srcRes.getResource().Get());

		// to track resources with temp life time only inside the command list
		trackResource(dstRes);
		trackResource(srcRes);
	}

	void CommandList::setGraphicsDynamicConstantBuffer(uint32_t rootParameterIndex, size_t sizeInBytes, const void* bufferData)
	{
		// const buffers must 256-bit aligned
		auto heapAllocation = m_uploadBuffer->allocate(sizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
		memcpy(heapAllocation.cpu, bufferData, sizeInBytes);
		m_commandList->SetGraphicsRootConstantBufferView(rootParameterIndex, heapAllocation.gpu);
	}

	void CommandList::trackResource(const Resource& res)
	{
		trackObject(res.getResource());
	}

	void CommandList::trackObject(Microsoft::WRL::ComPtr<ID3D12Object> object)
	{
		m_trackedObjects.push_back(object);
	}

	void CommandList::setShaderResourceView(
		uint32_t rootParameterIndex,
		uint32_t descriptorOffset,
		const Resource& resource,
		D3D12_RESOURCE_STATES stateAfter,
		UINT firstSubresource,
		UINT numSubresources,
		const D3D12_SHADER_RESOURCE_VIEW_DESC* srv)
	{
		if (numSubresources < D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
		{
			for (uint32_t i = 0; i < numSubresources; ++i)
			{
				transitionBarrier(resource, stateAfter, firstSubresource + i);
			}
		}
		else
		{
			transitionBarrier(resource, stateAfter);
		}

		m_dynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->stageDescriptors(rootParameterIndex, descriptorOffset, 1, resource.getShaderResourceView(srv));

		trackResource(resource);
	}

}
