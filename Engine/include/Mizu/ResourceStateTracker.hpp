#pragma once

#include "d3d12.h"

#include <mutex>
#include <map>
#include <unordered_map>
#include <vector>


class CommandList;
class Resource;


namespace Mizu
{
	class ResourceStateTracker
	{
	public:
		ResourceStateTracker()
		{

		}

		virtual ~ResourceStateTracker()
		{

		}

		// pushes a resource barrier into the the barrier tracker
		void resourceBarrier(const D3D12_RESOURCE_BARRIER& barrier)
		{

		}

		/**
		 * \brief pushed a transition resource barrier to the resource state tracker
		 * \param resource the resource to apply the transition on
		 * \param stateAfter the state to transit the resource to
		 * \param subresource the subresource to transition (default is all the subresources)
		 */
		void transitResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES stateAfter, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
		{

		}

		/**
		* \brief pushed a transition resource barrier to the resource state tracker
		* \param resource the resource to apply the transition on
		* \param stateAfter the state to transit the resource to
		* \param subresource the subresource to transition (default is all the subresources)
		*/
		void transitResource(const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
		{

		}

		/**
		 * \brief pushes a UAV resource barrier for a given resource
		 * \param resource the resource to add a UAV barrier for, nullptr is valid which indicates that any UAV access could require the barrier (TODO I did not get this one)
		 */
		void uavBarrier(const Resource* resource = nullptr)
		{
			
		}

		// TODO did not get this one also
		/**
		 * \brief push an alias barrier to a given resource
		 * \param resourceBefore the resource currently occupying the space in the heap
		 * \param resourceAfter the resource that will be occupying the space on the heap
		 * if any of parameters is null that means any placed or reserved resource could cause aliasing
		 */
		void aliasBarrier(const Resource* resourceBefore = nullptr, const Resource* resourceAfter = nullptr)
		{
			
		}
		// flush any pending resource barriers in the command list (should be called after the command list is called and before it is executed on the command queue)
		uint32_t flushPendingResourceBarriers(CommandList& commandList)
		{
			return 0;
		}

		// flush any (non-pending) resource barriers that have been pushed to the resource state tracker
		void flushResourceBarriers(CommandList& commandList)
		{
		}
	};
}