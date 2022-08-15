#include "CommandList.hpp"
#include "Resource.hpp"
#include "ResourceStateTracker.hpp"


namespace Mizu
{
	void CommandList::TransitionBarrier(const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource, bool flushBarriers)
	{
		auto pResource = resource.getResource();
		if(pResource)
		{
			// we don't care about the before state because the resource state tracker will fix it for us
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(pResource.Get(), D3D12_RESOURCE_STATE_COMMON, stateAfter, subResource);
			m_resourceStateTracker->resourceBarrier(barrier);
		}

		if(flushBarriers)
		{
			flushResourceBarriers();
		}
	}
}
