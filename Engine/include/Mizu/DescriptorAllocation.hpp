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
        DescriptorAllocation();

        DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE handle, uint32_t handlesCount, uint32_t descriptorSize, std::shared_ptr<DescriptorAllocatorPage> pDescriptorAllocatorPage);

    	~DescriptorAllocation();



        // ban copying
        DescriptorAllocation(const DescriptorAllocation&) = delete;
        DescriptorAllocation& operator=(const DescriptorAllocation&) = delete;

        // moving is allowed
        DescriptorAllocation(DescriptorAllocation&& allocation);
        DescriptorAllocation& operator=(DescriptorAllocation&& other);



        bool isNull() const
        {
            return m_descriptor.ptr == 0;
        }

        uint32_t getHandlesCount() const
        {
            return m_handlesCount;
        }

        std::shared_ptr<DescriptorAllocatorPage> getDescriptorAllocatorPage() const;

        D3D12_CPU_DESCRIPTOR_HANDLE getDescriptorHandle(uint32_t offset = 0) const;

    private:

        void freeAllocation();

        D3D12_CPU_DESCRIPTOR_HANDLE m_descriptor;

        uint32_t m_handlesCount;

        uint32_t m_descriptorSize;


        std::shared_ptr<DescriptorAllocatorPage> m_page;
    };
}