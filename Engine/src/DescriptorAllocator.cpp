#include "Helpers.h"

#include "DescriptorAllocator.hpp"
#include "DescriptorAllocatorPage.hpp"

#include <algorithm>

namespace Mizu
{
    DescriptorAllocator::DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t descriptorsPerHeap)
     : m_heapType(type), m_descriptorsPerHeap(descriptorsPerHeap)
    {

    }

    std::shared_ptr<DescriptorAllocatorPage> DescriptorAllocator::createDescriptorAllocatorPage()
    {
        auto newPage = std::make_shared<DescriptorAllocatorPage>(m_heapType, m_descriptorsPerHeap);

        m_heapPool.push_back(newPage);
        m_availableHeaps.insert(m_heapPool.size() - 1);

        return newPage;
    }

    DescriptorAllocation DescriptorAllocator::allocate(uint32_t n)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        DescriptorAllocation allocation;

        for(auto it = m_availableHeaps.begin(); it != m_availableHeaps.end(); it++)
        {
            auto allocatorPage = m_heapPool[*it];

            allocation = allocatorPage->allocateFromPage(n);

            if (!allocatorPage->freeHandlesCount())
                it = m_availableHeaps.erase(it);

            // found a vaild allocation
            // TODO maybe make bool operator
            if (allocation.isNull())
                break;
        }

        // no heap satisfying the conditions could be found
        if(allocation.isNull())
        {
            // extend the size
            m_descriptorsPerHeap = std::max(m_descriptorsPerHeap, n);
            // make a new page of the size needed
            auto newPage = createDescriptorAllocatorPage();

            allocation = newPage->allocateFromPage(n);
        }

        return allocation;
    }

    void DescriptorAllocator::releaseStaleDescriptors(uint64_t frameNumber)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        for(size_t i = 0; i < m_heapPool.size();i++)
        {
            auto& page = m_heapPool[i];

            page->releaseStaleDescriptors(frameNumber);

            if (page->freeHandlesCount())
                m_availableHeaps.insert(i);
        }
    }

}
