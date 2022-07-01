#pragma once

#include "d3dx12.h"


namespace Mizu 
{
    class DescriptorAllocatorPage;

    // TODO this is temp just to make compile
    class DescriptorAllocation
    {
    public:

        // creates a null descriptor (default just a place holder to compile)
        DescriptorAllocation() = default;

        DescriptorAllocation(CD3DX12_CPU_DESCRIPTOR_HANDLE handle, uint32_t handlesCount, uint32_t descriptorSize, std::shared_ptr<DescriptorAllocatorPage> pDescriptorAllocatorPage);


    	~DescriptorAllocation();



        // ban copying
        DescriptorAllocation(const DescriptorAllocation&) = delete;
        DescriptorAllocation& operator=(const DescriptorAllocation&) = delete;

        // moving is allowed
        DescriptorAllocation(const DescriptorAllocation&& allocation);
        DescriptorAllocation& operator=(const DescriptorAllocation&& other);



        bool isNull() const
        {
            return true;
        }

        uint32_t getFreeHandlesCount()
        {
            return 0;
        }

        //CD3DX12_CPU_DESCRIPTOR_HANDLE getDescriptorHandle(uint32_t offset) const
        //{
	       // 
        //}

        uint32_t getHandlesCount() const
        {
            return 0;
        }

        std::shared_ptr<DescriptorAllocatorPage> GetDescriptorAllocatorPage() const
        {
            return nullptr;
        }

    };
}