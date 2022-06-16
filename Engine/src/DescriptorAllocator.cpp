#include "DescriptorAllocator.hpp"

namespace Mizu
{
    // TODO next time
    DescriptorAllocator::DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t descriptorsPerHeap)
     : m_heapType(type), m_descriptorsPerHeap(descriptorsPerHeap)
    {

    }

    std::shared_ptr<DescriptorAllocatorPage> DescriptorAllocator::createDescriptorAllocatorPage()
    {
        return nullptr;
    }

    DescriptorAllocation DescriptorAllocator::allocate(uint32_t n)
    {
        return DescriptorAllocation();
    }

    void DescriptorAllocator::releaseStaleDescriptors(uint64_t frameNumber)
    {
        
    }

}
