#include "DescriptorAllocation.hpp"
#include "DescriptorAllocatorPage.hpp"

namespace Mizu
{
    DescriptorAllocation::DescriptorAllocation(CD3DX12_CPU_DESCRIPTOR_HANDLE handle, uint32_t handlesCount, uint32_t descriptorSize, std::shared_ptr<DescriptorAllocatorPage> pDescriptorAllocatorPage)
    {
        
    }

    Mizu::DescriptorAllocation::~DescriptorAllocation()
    {
    }

    Mizu::DescriptorAllocation::DescriptorAllocation(const DescriptorAllocation&& allocation)
    {
    }

    Mizu::DescriptorAllocation& Mizu::DescriptorAllocation::operator=(const DescriptorAllocation&& other)
    {
        return DescriptorAllocation();
    }
}