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
			else // the resource is being used on the command list for the first time
			{
				// add a pending barrier that will be resolved before the command list is executed on the command queue
				m_pendingResourceBarriers.push_back(barrier);
			}

			// push the final known state (possibly replacing the last known state for the subresource)
			m_finalResourceState[transitionBarrier.pResource].setSubresourceState(transitionBarrier.Subresource, transitionBarrier.StateAfter);
		}
		else
		{
			// just push the non-transition barriers to the resource barriers array
			m_resourceBarriers.push_back(barrier);
		}
	}

	void ResourceStateTracker::transitResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES stateAfter, UINT subresource)
	{
		if(resource)
		{
			resourceBarrier(CD3DX12_RESOURCE_BARRIER::Transition(resource, D3D12_RESOURCE_STATE_COMMON, stateAfter, subresource));
		}
	}

	void ResourceStateTracker::transitResource(const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subresource)
	{
		transitResource(resource.getResource().Get(), stateAfter, subresource);
	}

	void ResourceStateTracker::uavBarrier(const Resource* resource)
	{
		ID3D12Resource* pResource = resource != nullptr ? resource->getResource().Get() : nullptr;

		// if pResource is nullptr that means all the UAV operations must finish before any UAV operation can be performed (it might cause stalling so this is only for synchronizing read and write)
		resourceBarrier(CD3DX12_RESOURCE_BARRIER::UAV(pResource));
	}

	void ResourceStateTracker::aliasBarrier(const Resource* resourceBefore, const Resource* resourceAfter)
	{
		ID3D12Resource* pResourceBefore = resourceBefore != nullptr ? resourceBefore->getResource().Get() : nullptr;
		ID3D12Resource* pResourceAfter = resourceAfter != nullptr ? resourceAfter->getResource().Get() : nullptr;

		resourceBarrier(CD3DX12_RESOURCE_BARRIER::Aliasing(pResourceBefore, pResourceAfter));
	}
	uint32_t ResourceStateTracker::flushPendingResourceBarriers(CommandList& commandList)
	{
		assert(ms_isLocked);
		// resolve the pending resource barriers by checking if global state of the (sub)resource != pending state then it should be added to the intermediate command list
		ResourceBarriers resourceBarriers;

		resourceBarriers.reserve(m_pendingResourceBarriers.size());

		for(auto pb : m_pendingResourceBarriers)
		{
			if(pb.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION) // because only transit barriers should be pending
			{
				auto pendingTransition = pb.Transition;

				const auto& it = ms_globalResourceState.find(pendingTransition.pResource);
				if(it != ms_globalResourceState.end())
				{
					auto& resourceState = it->second;
					if(pendingTransition.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES && !resourceState.subresourceState.empty()) // todo I think empty check here is not needed
					{
						// Transit all the subresources
						for(auto subresourceState : resourceState.subresourceState)
						{
							if(pendingTransition.StateAfter != subresourceState.second)
							{
								D3D12_RESOURCE_BARRIER newBarrier = pb;// = pending barrier
								newBarrier.Transition.Subresource = subresourceState.first;
								newBarrier.Transition.StateBefore = subresourceState.second;
								resourceBarriers.push_back(newBarrier);
							}
						}
					}
					else
					{
						// No (sub)resources need to be transitioned. Just add a single transition barrier (if needed).
						auto globalState = (it->second).getSubresourceState(pendingTransition.Subresource);
						if (pendingTransition.StateAfter != globalState)
						{
							// Fix-up the before state based on current global state of the resource.
							pb.Transition.StateBefore = globalState;
							resourceBarriers.push_back(pb);
						}
					}
				}
			}
		}

		UINT numBarriers = static_cast<UINT>(resourceBarriers.size());
		if (numBarriers > 0)
		{
			auto cmdList = commandList.getGraphicsCommandList();
			cmdList->ResourceBarrier(numBarriers, resourceBarriers.data());
		}

		m_pendingResourceBarriers.clear();

		return numBarriers;
	}

	void ResourceStateTracker::flushResourceBarriers(CommandList& commandList)
	{
		UINT barriesCount = static_cast<UINT>(m_resourceBarriers.size());

		if (barriesCount == 0)
			return;

		auto cmdList = commandList.getGraphicsCommandList();
		cmdList->ResourceBarrier(barriesCount, m_resourceBarriers.data());
		m_resourceBarriers.clear();
	}

	void ResourceStateTracker::commitFinalResourceStates()
	{
		assert(ms_isLocked);

		// commit the final resource states to the global resource state array
		for(const auto& [idx, val] : m_finalResourceState)
		{
			ms_globalResourceState[idx] = val;
		}

		m_finalResourceState.clear();
	}

	// called when the command list is reset
	void ResourceStateTracker::resetState()
	{
		m_pendingResourceBarriers.clear();
		m_resourceBarriers.clear();
		m_finalResourceState.clear();
	}

	// (static)
	void ResourceStateTracker::lock()
	{
		ms_mutex.lock();
		ms_isLocked = true;
	}

	// (static)
	void ResourceStateTracker::unlock()
	{
		ms_isLocked = false;
		ms_mutex.unlock();
	}

	// (static) called whenever a new resource is created using (createCommittedResource, createPlacedResource, CreateReservedResource)
	void ResourceStateTracker::addGlobalResourceState(ID3D12Resource* resource, D3D12_RESOURCE_STATES state)
	{
		if(resource)
		{
			std::lock_guard<std::mutex> lock(ms_mutex);
			ms_globalResourceState[resource].setSubresourceState(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, state);
		}
	}

	// (static) removes resource state from the global state map
	void ResourceStateTracker::removeGlobalResourceState(ID3D12Resource* resource)
	{
		if(resource)
		{
			std::lock_guard<std::mutex> lock(ms_mutex);
			ms_globalResourceState.erase(resource);
		}
	}
}