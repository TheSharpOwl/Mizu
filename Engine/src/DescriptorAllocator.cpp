#include "DescriptorAllocator.hpp"
#include "DescriptorAllocatorPage.hpp"
#include "DX12LibPCH.h"


namespace Mizu
{

    DescriptorAllocator::DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptorsPerHeap) :
            m_heapType(type), m_numDescriptorsPerHeap(numDescriptorsPerHeap)
    {}

    DescriptorAllocator::~DescriptorAllocator()
    {

    }

    DescriptorAllocation DescriptorAllocator::allocate(uint32_t numDescriptors)
    {
        std::lock_guard<std::mutex> lock(m_allocationMutex);

        DescriptorAllocation allocation;
        for (auto pIndex = m_availableHeaps.begin(); pIndex != m_availableHeaps.end(); pIndex++) {
            auto allocatorPage = m_heapPool[*pIndex];

            allocation = allocatorPage->allocate(numDescriptors);

            if (allocatorPage->numFreeHandles() == 0) {
                pIndex = m_availableHeaps.erase(pIndex);
            }

            // we found a valid allocation
            if (!allocation.isNull()) {
                break;
            }
        }

        if (allocation.isNull()) {
            m_numDescriptorsPerHeap = std::max(m_numDescriptorsPerHeap, numDescriptors);
            auto newPage = createAllocatorPage();

            allocation = newPage->allocate(numDescriptors);
        }

        return allocation;
    }

    void DescriptorAllocator::releaseStaleDescriptors(uint64_t frameNum)
    {
        std::lock_guard<std::mutex> lock(m_allocationMutex);

        int i = 0;
        for (auto page : m_heapPool) {
            page->releaseStaleDescriptors(frameNum);

            if (page->numFreeHandles() > 0) {
                m_availableHeaps.insert(i);
            }
            i++;
        }
    }

    std::shared_ptr<DescriptorAllocatorPage> DescriptorAllocator::createAllocatorPage()
    {
        auto newPage = std::make_shared<DescriptorAllocatorPage>(m_heapType, m_numDescriptorsPerHeap);

        m_heapPool.emplace_back(newPage);
        m_availableHeaps.insert(m_heapPool.size() - 1);

        return newPage;
    }
}
