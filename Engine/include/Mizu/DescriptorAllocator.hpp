#pragma once

// TODO
#include "DescriptorAllocation.hpp"

#include "d3dx12.h"

#include <cstdint>
#include <mutex>
#include <memory>
#include <set>
#include <vector>

namespace Mizu
{
    class DescriptorAllocatorPage;

    class DescriptorAllocator
    {
    public:
        DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t descriptorsPerHeap = 256);

        virtual ~DescriptorAllocator() = default;

        /**
         * @brief Allocates n descriptors and cannot be more than the number of descriptors per descriptor heap
         *
         * @param n number of descriptors/views to allocate
         * @return DescriptorAllocation
         */
        DescriptorAllocation allocate(uint32_t n);

        void releaseStaleDescriptors(uint64_t frameNumber);

    private:
        using DescriptorHeapPool = std::vector<std::shared_ptr<DescriptorAllocatorPage>>;

        std::shared_ptr<DescriptorAllocatorPage> createDescriptorAllocatorPage();

        D3D12_DESCRIPTOR_HEAP_TYPE m_heapType;

        uint32_t m_descriptorsPerHeap;

        DescriptorHeapPool m_heapPool;

        std::set<size_t> m_availableHeaps;

        std::mutex m_mutex;
    };
}
