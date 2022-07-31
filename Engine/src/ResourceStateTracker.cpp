#include "Mizu/ResourceStateTracker.hpp"
#include "Mizu/CommandList.hpp"
#include "Mizu/Resource.hpp"

#include "DX12LibPCH.h"

namespace Mizu
{
	ResourceStateTracker::ResourceStateTracker()
	{

	}

	ResourceStateTracker::~ResourceStateTracker()
	{

	}

	void ResourceStateTracker::resourceBarrier(const D3D12_RESOURCE_BARRIER& barrier)
	{

	}

	void ResourceStateTracker::transitResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES stateAfter, UINT subresource)
	{

	}

	void ResourceStateTracker::transitResource(const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subresource)
	{

	}

	void ResourceStateTracker::uavBarrier(const Resource* resource)
	{

	}

	void ResourceStateTracker::aliasBarrier(const Resource* resourceBefore, const Resource* resourceAfter)
	{

	}
	uint32_t ResourceStateTracker::flushPendingResourceBarriers(CommandList& commandList)
	{
		return 0;
	}

	void ResourceStateTracker::flushResourceBarriers(CommandList& commandList)
	{
	}

	void ResourceStateTracker::commitFinalResourceStates()
	{

	}

	void ResourceStateTracker::resetState() {}

	// (static)
	void ResourceStateTracker::lock() {}

	// (static)
	void ResourceStateTracker::unlock() {}

	// (static)
	void ResourceStateTracker::addGlobalResourceState(ID3D12Resource* resource, D3D12_RESOURCE_STATES state) {}

	// (static)
	void ResourceStateTracker::removeGlobalResourceState(ID3D12Resource* resource) {}
}