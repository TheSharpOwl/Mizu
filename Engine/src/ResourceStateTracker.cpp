#include "Mizu/ResourceStateTracker.hpp"
#include "Mizu/CommandList.hpp"
#include "Mizu/Resource.hpp"

#include "DX12LibPCH.h"



namespace Mizu
{
	// static definitions
	std::mutex ResourceStateTracker::ms_mutex;
	bool ResourceStateTracker::ms_isLocked = false;
	ResourceStateTracker::ResourceStateMap ResourceStateTracker::ms_globalResourceState;

	ResourceStateTracker::ResourceStateTracker()
	{

	}

	ResourceStateTracker::~ResourceStateTracker()
	{

	}

	void ResourceStateTracker::resourceBarrier(const D3D12_RESOURCE_BARRIER& barrier)
	{
		if(barrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION)
		{
			const D3D12_RESOURCE_TRANSITION_BARRIER& transitionBarrier = barrier.Transition;

			// check first if the resource already a known "final" state for the given resource
			// because if there is that means the resource have been used on the command list before and has an already known state within the command list execution

			const auto it = m_finalResourceState.find(transitionBarrier.pResource);

			if(it != m_finalResourceState.end())
			{
				auto& resourceState = it->second;
				// check if the final known state of the resource is different
				if(transitionBarrier.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES && !resourceState.subresourceState.empty())
				{
					// transit all the subresources if they are different from the final state
					for(auto subresourceState : resourceState.subresourceState)
					{
						if( transitionBarrier.StateAfter != subresourceState.second)
						{
							D3D12_RESOURCE_BARRIER newBarrier = barrier;
							newBarrier.Transition.Subresource = subresourceState.first; // specify that it's about this subresource
							newBarrier.Transition.StateBefore = subresourceState.second; // specify the state before
							m_resourceBarriers.push_back(newBarrier);
						}
					}
				}
				else
				{
					auto finalState = resourceState.getSubresourceState(transitionBarrier.Subresource);
					if (transitionBarrier.StateAfter != finalState)
					{
						// Push a new transition barrier with the correct before state.
						D3D12_RESOURCE_BARRIER newBarrier = barrier;
						newBarrier.Transition.StateBefore = finalState;
						m_resourceBarriers.push_back(newBarrier);
					}
				}

			}
			else
			{
				
			}
		}
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