
#include "DescriptorAllocatorPage.hpp"

#include <DX12LibPCH.h>
#include "Application.hpp"

namespace Mizu
{
    DescriptorAllocatorPage::DescriptorAllocatorPage(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numOfDescriptors) :
    m_heapType(type),
    m_numDescriptorsHeap(numOfDescriptors)
    {
        auto device = Application::Get().GetDevice();

        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.Type = m_heapType;
        heapDesc.NumDescriptors = m_numDescriptorsHeap;

        ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_descriptorHeap)));

        m_baseDescriptor = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
        m_descriptorHandleIncrementSize = device->GetDescriptorHandleIncrementSize(m_heapType);
        m_numFreeHandles = m_numDescriptorsHeap;


        // initialize the free lists
        addNewblock(0, m_numFreeHandles);
    }

    D3D12_DESCRIPTOR_HEAP_TYPE DescriptorAllocatorPage::getHeapType()
    {
        return m_heapType;
    }

    bool DescriptorAllocatorPage::hasSpace(uint32_t numDescriptors) const
    {
        return m_freeListBySize.lower_bound(numDescriptors) != m_freeListBySize.end();
    }

    uint32_t DescriptorAllocatorPage::numFreeHandles() const
    {
        return m_numFreeHandles;
    }

    void DescriptorAllocatorPage::addNewblock(uint32_t offset, uint32_t numDescriptors)
    {
        auto offsetIt = m_freelistByOffset.emplace(offset, numDescriptors);
        auto sizeIt = m_freeListBySize.emplace(numDescriptors, offsetIt.first);
        offsetIt.first->second.FreeListBySize = sizeIt;

    }

    DescriptorAllocation DescriptorAllocatorPage::allocate(uint32_t numDescriptors)
    {
        std::lock_guard<std::mutex>(m_mutex);

        if (numDescriptors > m_numFreeHandles)
        {
            return DescriptorAllocation();
        }

        // get the first block which is large enough to satisfy the request
        auto smallestBlockIt = m_freeListBySize.lower_bound(numDescriptors);
        // TODO write why here for better explanation
        // still we might not find a suitable one
        if(smallestBlockIt == m_freeListBySize.end())
        {
            return DescriptorAllocation();
        }

        auto blockSize = smallestBlockIt->first;

        auto offsetIt = smallestBlockIt->second;

        auto offset = offsetIt->first;

        m_freeListBySize.erase(smallestBlockIt);
        m_freelistByOffset.erase(offsetIt);

        auto newOffset = offset + numDescriptors;
        auto newSize = blockSize - numDescriptors;

        if(newSize > 0)
        {
            // if the allocation didn't match exactly the requested size, return the left over to the free size
            addNewblock(newOffset, newSize);
        }

        m_numFreeHandles -= numDescriptors;
        return DescriptorAllocation(CD3DX12_CPU_DESCRIPTOR_HANDLE(m_baseDescriptor, offset, m_descriptorHandleIncrementSize),
                                    numDescriptors, m_descriptorHandleIncrementSize, shared_from_this());

    }

    uint32_t DescriptorAllocatorPage::computeOffset(D3D12_CPU_DESCRIPTOR_HANDLE handle)
    {
        return static_cast<uint32_t>(handle.ptr - m_baseDescriptor.ptr) / m_descriptorHandleIncrementSize;
    }

    void DescriptorAllocatorPage::free( DescriptorAllocation&& descriptor, uint64_t frameNumber )
    {
        // Compute the offset of the descriptor within the descriptor heap.
        auto offset = ComputeOffset( descriptor.getDescriptorHandle() );

        std::lock_guard<std::mutex> lock( m_mutex );

        // Don't add the block directly to the free list until the frame has completed.
        m_staleDescriptors.emplace( offset, descriptor.getNumHandles(), frameNumber );
    }

    void DescriptorAllocatorPage::freeBlock(uint32_t offset, uint32_t numDescriptors)
    {
        auto nextBlockIt = m_freeListBySize.upper_bound(offset);

        auto prevBlockIt = nextBlockIt;

        if(prevBlockIt != m_freeListBySize.begin())
        {
            prevBlockIt--;
        }
        else
        {
            prevBlockIt = m_freeListBySize.end();
        }

        m_numFreeHandles += numDescriptors;


        if ( prevBlockIt != m_FreeListByOffset.end() &&
             offset == prevBlockIt->first + prevBlockIt->second.Size )
        {
            // The previous block is exactly behind the block that is to be freed.
            // Increase the block size by the size of merging with the previous block.
            offset = prevBlockIt->first;
            numDescriptors += prevBlockIt->second.Size;

            // Remove the previous block from the free list.
            m_FreeListBySize.erase( prevBlockIt->second.FreeListBySizeIt );
            m_FreeListByOffset.erase( prevBlockIt );
        }

        if ( nextBlockIt != m_FreeListByOffset.end() &&
             offset + numDescriptors == nextBlockIt->first )
        {
            // The next block is exactly in front of the block that is to be freed.
            //
            // Offset               NextBlock.Offset
            // |                    |
            // |<------Size-------->|<-----NextBlock.Size----->|

            // Increase the block size by the size of merging with the next block.
            numDescriptors += nextBlockIt->second.Size;

            // Remove the next block from the free list.
            m_FreeListBySize.erase( nextBlockIt->second.FreeListBySizeIt );
            m_FreeListByOffset.erase( nextBlockIt );
        }

        // Add the freed block to the free list.
        addNewBlock( offset, numDescriptors );
    }


    void DescriptorAllocatorPage::releaseStaleDescriptors( uint64_t frameNumber )
    {
        std::lock_guard<std::mutex> lock( m_mutex );

        while ( !m_staleDescriptors.empty() && m_staleDescriptors.front().frameNumber <= frameNumber )
        {
            auto& staleDescriptor = m_staleDescriptors.front();

            // The offset of the descriptor inside the heap.
            auto offset = staleDescriptor.Offset;
            // The number of descriptors that were allocated.
            auto numDescriptors = staleDescriptor.Size;

            freeBlock( offset, numDescriptors );

            m_staleDescriptors.pop();
        }
    }
}