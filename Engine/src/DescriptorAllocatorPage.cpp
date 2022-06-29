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

    // n is the number of descriptors
    DescriptorAllocation DescriptorAllocatorPage::allocateFromPage(uint32_t n)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (n > m_freeHandlesCount)
        {
            return DescriptorAllocation();
        }

        auto sizeIt = m_freeListBySize.lower_bound(n);

        if(sizeIt == m_freeListBySize.end())
        {
            return DescriptorAllocation();
        }

        auto offsetIt = sizeIt->second;

        const auto offset = offsetIt->first;

        auto blockInfo = offsetIt->second;

        // In case there is a piece left in the map return it
        const OffsetType offsetAfter = offsetIt->first + n;
        const SizeType sizeLeft = sizeIt->first - n;

        // remove data from the maps because we will take it
        m_freeListBySize.erase(sizeIt);
        m_freelistByoffset.erase(offsetIt);

        // can't be less than zero
        if(sizeLeft)
        {
            addNewBlock(offsetAfter, sizeLeft);
        }

        // TODO understand this part after coding the descriptor allocation
    	return DescriptorAllocation(
            CD3DX12_CPU_DESCRIPTOR_HANDLE(m_baseDescriptor, offset, m_descriptorHandleIncreamentSize),
            n, m_descriptorHandleIncreamentSize, shared_from_this());

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
        // Note that emplace returns pair<iterator,bool> so .first is the iterator (IF IT'S MAP NOT MULTIMAP because multimap is always successful to insert into)
        auto [offsetIt, _] = m_freelistByoffset.emplace(offset, descriptorsCount); // offset is the key and FreeBlockInfo(descriptorsCount) is the value
        const auto sizeIt = m_freeListBySize.emplace(descriptorsCount, offsetIt);
        // add reference to the sizeIt in the offset freeList (in 
        offsetIt->second.freeListBySizeIt = sizeIt;
    }

    void DescriptorAllocatorPage::freeBlock(uint32_t offset, uint32_t descriptorsCount)
    {

    }

    uint32_t DescriptorAllocatorPage::getFreeHandlesCount() const
    {
        return m_freeHandlesCount;
    }
}