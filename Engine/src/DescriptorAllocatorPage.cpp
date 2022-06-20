#include "DescriptorAllocatorPage.hpp"

// TODO tomorrow with understanding why it has shared from this... (it is described in the next class session)

namespace Mizu
{
    bool DescriptorAllocatorPage::hasSpace() const
    {
        return false;
    }

    int DescriptorAllocatorPage::freeHandlesCount() const
    {
        return 0;
    }

    D3D12_DESCRIPTOR_HEAP_TYPE DescriptorAllocatorPage::getHeapType() const
    {
        return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    }

    DescriptorAllocation DescriptorAllocatorPage::allocate(uint32_t n)
    {
        return DescriptorAllocation();
    }

    void DescriptorAllocatorPage::free(DescriptorAllocation&& descriptorHandle, uint64_t frameNum)
    {

    }

    void DescriptorAllocatorPage::releaseStaleDescriptors(uint64_t frameNum)
    {

    }

    uint32_t DescriptorAllocatorPage::computeOffset(D3D12_CPU_DESCRIPTOR_HANDLE handle)
    {
        return 0;
    }

    void DescriptorAllocatorPage::addNewBlock(uint32_t offset, uint32_t descriptorsCount)
    {
	    
    }

    void DescriptorAllocatorPage::freeBlock(uint32_t offset, uint32_t descriptorsCount)
    {
	    
    }
}