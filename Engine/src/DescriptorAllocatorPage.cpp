#include "DescriptorAllocatorPage.hpp"

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
}