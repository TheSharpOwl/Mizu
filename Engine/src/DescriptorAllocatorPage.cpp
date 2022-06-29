#include "DescriptorAllocatorPage.hpp"
#include "Application.hpp"

// TODO tomorrow with understanding why it has shared from this... (it is described in the next class session)

namespace Mizu
{
    DescriptorAllocatorPage::DescriptorAllocatorPage(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors) :
    m_heapType(type), m_descriptorsInHeapCount(numDescriptors)
    {
        auto device = Application::Get().GetDevice();

        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.Type = m_heapType;
        heapDesc.NumDescriptors = m_descriptorsInHeapCount;

        ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_descriptorHeap)));

        m_baseDescriptor = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
        m_descriptorHandleIncreamentSize = device->GetDescriptorHandleIncrementSize(m_heapType);
        m_freeHandlesCount = m_descriptorsInHeapCount;

        addNewBlock(0, m_freeHandlesCount);
    }

    bool DescriptorAllocatorPage::hasSpace(uint32_t descriptorsCount) const
    {
        // TODO check maybe the mutex should be locked here? (or because it is only called internally?!)
        return m_freeListBySize.lower_bound(descriptorsCount) != m_freeListBySize.end();
    }

    int DescriptorAllocatorPage::freeHandlesCount() const
    {
        return 0;
    }

    D3D12_DESCRIPTOR_HEAP_TYPE DescriptorAllocatorPage::getHeapType() const
    {
        return m_heapType;
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
        // Note that emplace returns pair<iterator,bool> so .first is the iterator
        auto [offsetIt, success] = m_freelistByoffset.emplace(offset, descriptorsCount); // offset is the key and FreeBlockInfo(descriptorsCount) is the value
        auto sizeIt = m_freeListBySize.emplace(descriptorsCount, offsetIt);
        // add reference to the sizeIt in the offset freeList (in 
        offsetIt->second.freeListBySizeIt = sizeIt;
    }

    void DescriptorAllocatorPage::freeBlock(uint32_t offset, uint32_t descriptorsCount)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if(descriptorsCount > m_freeHandlesCount)
    }

    uint32_t DescriptorAllocatorPage::getFreeHandlesCount() const
    {
        return m_freeHandlesCount;
    }
}