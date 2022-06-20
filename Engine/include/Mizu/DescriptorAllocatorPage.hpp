#include <d3d12.h>

#include <memory>

#include "DescriptorAllocation.hpp"

// TODO this header file just contains the functions and returning random values just to make sure it compiles so that I can push a compiling code and relax :3
namespace Mizu
{
    class DescriptorAllocatorPage : public std::enable_shared_from_this<DescriptorAllocatorPage>
    {
    public:

        DescriptorAllocatorPage(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors) {}
        
        bool hasSpace() const;
        int freeHandlesCount() const;

        D3D12_DESCRIPTOR_HEAP_TYPE getHeapType() const;

        DescriptorAllocation allocate(uint32_t n);

        void free(DescriptorAllocation&& descriptorHandle, uint64_t frameNum);

        void releaseStaleDescriptors(uint64_t frameNum);

    protected:
    private:
        // TODO tomorrow with understanding why it has shared from this...
    };
}