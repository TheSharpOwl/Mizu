// TODO fix it after adding to cmake
#include "Mizu/DescriptorAllocator.hpp"

namespace Mizu {

	DescriptorAllocator::DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptorsPerHeap)
	{
	}

	DescriptorAllocator::~DescriptorAllocator()
	{
	}

	DescriptorAllocation DescriptorAllocator::allocate(uint32_t numDescriptors)
	{
		return DescriptorAllocation();
	}

	void DescriptorAllocator::releaseStaleDescriptors(uint64_t frameNum)
	{
	}
}
