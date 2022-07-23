#include <DX12LibPCH.h>

#include <memory>
#include <map>
#include <queue>
#include <mutex>

#include "DescriptorAllocation.hpp"

// TODO this header file just contains the functions and returning random values just to make sure it compiles so that I can push a compiling code and relax :3
namespace Mizu
{
    class DescriptorAllocatorPage : public std::enable_shared_from_this<DescriptorAllocatorPage>
    {
    public:

        DescriptorAllocatorPage(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors);
        
        bool hasSpace(uint32_t descriptorsCount) const;
        int freeHandlesCount() const;

        D3D12_DESCRIPTOR_HEAP_TYPE getHeapType() const;

        DescriptorAllocation allocateFromPage(uint32_t n);

        void freeDescriptor(DescriptorAllocation&& descriptorHandle, uint64_t frameNum);

        void releaseStaleDescriptors(uint64_t frameNum);

        uint32_t getFreeHandlesCount() const;

    protected:

        // offset from the start of the heap
        uint32_t computeOffset(D3D12_CPU_DESCRIPTOR_HANDLE handle);

        void addNewBlock(uint32_t offset, uint32_t descriptorsCount);

        void freeBlock(uint32_t offset, uint32_t descriptorsCount);

    private:

        // for better readability
        using OffsetType = uint32_t;

        using SizeType = uint32_t;


        struct FreeBlockInfo;

        // get the list using the offset from the start
        using FreeListByOffset = std::map<OffsetType, FreeBlockInfo>;

        // multi because the size is the key (many blocks can have the same size)
        // also points to the position in the first map (By Offset map)
        using FreeListBySize = std::multimap<SizeType, FreeListByOffset::iterator>;

        struct FreeBlockInfo
        {
            FreeBlockInfo(SizeType size) : _size(size) {}

            // size as a name will give me trouble later so _size is a better name
            SizeType _size;
            // iterator on a multi map with <SizeType, iterator on free list by offset>
            FreeListBySize::iterator freeListBySizeIt;
        };

        // to keep track of descriptors in the descriptor heap that are freed but still in use so we cannot release them yet until their frame is finished on the cpu
        struct StaleDescriptorInfo
        {
			StaleDescriptorInfo(OffsetType offset, SizeType size, uint64_t frame)
				:offset(offset),
				_size(size),
				frameNumber(frame)
			{}

            OffsetType offset;
            SizeType _size;
            uint64_t frameNumber;
        };

        using StaleDescriptorQueue = std::queue<StaleDescriptorInfo>;

        FreeListByOffset m_freelistByoffset;
        FreeListBySize m_freeListBySize;
        StaleDescriptorQueue  m_staleDescriptors;

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;
        D3D12_DESCRIPTOR_HEAP_TYPE m_heapType;
        CD3DX12_CPU_DESCRIPTOR_HANDLE m_baseDescriptor;

        uint32_t m_descriptorHandleIncreamentSize;
        uint32_t m_descriptorsInHeapCount;
        uint32_t m_freeHandlesCount;

        std::mutex m_mutex;
    };
}