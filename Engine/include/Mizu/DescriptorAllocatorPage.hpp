#pragma once

#include "DescriptorAllocatorPage.hpp"

#include "d3d12.h"
#include "DescriptorAllocation.hpp"

#include <wrl.h>

#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <d3dx12.h>

namespace Mizu
{
    class DescriptorAllocatorPage
            : std::enable_shared_from_this<DescriptorAllocatorPage>
    {
    public:
        DescriptorAllocatorPage(D3D12_DESCRIPTOR_HEAP_TYPE type,
                                uint32_t numOfDescriptors);

        D3D12_DESCRIPTOR_HEAP_TYPE getHeapType();

        bool hasSpace(uint32_t numDescriptors) const;

        uint32_t numFreeHandles() const;


        DescriptorAllocation allocate(uint32_t numDescriptors);

        void free(DescriptorAllocation&& descriptorHandle, uint64_t frameNumber);

        void releaseStaleDescriptors(uint64_t frameNumber);

    protected:


        uint32_t computeOffset(D3D12_CPU_DESCRIPTOR_HANDLE handle);

        void addNewblock(uint32_t offset, uint32_t numDescriptors);

        // also merges 2 free neighbouring blocks if they exist
        void freeBlock(uint32_t offset, uint32_t numDescriptors);

    private:
        using offsetType = uint32_t;

        using sizeType = uint32_t;

        struct FreeBlockInfo;

        using FreeListByOffSet = std::map<offsetType, FreeBlockInfo>;

        // TODO read about multimap
        using FreeListBySize = std::multimap<sizeType, FreeListByOffSet::iterator>;

        struct FreeBlockInfo {
            FreeBlockInfo(size_t size) :
            size(size) {}

            sizeType size;

            FreeListBySize::iterator FreeListBySize;
        };

        struct StaleDescriptorInfo
        {
            StaleDescriptorInfo(offsetType offset, sizeType size, uint64_t frame) :
                    offset(offset),
                    size(size),
                    frameNumber(frame)
            {}
            // offset within the descriptor heap
            offsetType offset;
            // number of descriptors
            sizeType size;
            // frame number when the descriptor was freed
            uint64_t frameNumber;
        };

        using StaleDescriptorQueue = std::queue<StaleDescriptorInfo>;

        FreeListByOffSet m_freelistByOffset;
        FreeListBySize  m_freeListBySize;
        StaleDescriptorQueue m_staleDescriptors;

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;
        D3D12_DESCRIPTOR_HEAP_TYPE m_heapType;
        CD3DX12_CPU_DESCRIPTOR_HANDLE m_baseDescriptor;
        uint32_t m_descriptorHandleIncrementSize;
        uint32_t m_numDescriptorsHeap;
        uint32_t m_numFreeHandles;

        std::mutex m_mutex;
    };
}
