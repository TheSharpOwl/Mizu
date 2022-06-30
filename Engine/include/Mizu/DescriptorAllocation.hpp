#pragma once

#include "d3dx12.h"


namespace Mizu 
{
    class DescriptorAllocatorPage;

    // TODO this is temp just to make compile
    class DescriptorAllocation
    {
    public:
        DescriptorAllocation() = default;

        DescriptorAllocation(CD3DX12_CPU_DESCRIPTOR_HANDLE handle, uint32_t descriptorsCount, uint32_t descriptorIncrementSize, std::shared_ptr<DescriptorAllocatorPage> pDescriptorAllocatorPage);

        bool isNull()
        {
            return true;
        }

        uint32_t getFreeHandlesCount()
        {
            return 0;
        }


    };
}