#include "Mizu/DescriptorAllocation.hpp"

#include "d3dx12.h"

#include<memory>
#include<vector>
#include<set>
#include<mutex>

namespace Mizu {

	class DescrptorAllocatorPage;

	class DescriptorAllocator
	{
	public:
		DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptorsPerHeap = 256);

		virtual ~DescriptorAllocator();


		DescriptorAllocation allocate(uint32_t numDescriptors = 1);

		void releaseStaleDescriptors(uint64_t frameNum);

	private:
		// TODO maybe find a better name
		using DescriptorHeapPool = std::vector<std::shared_ptr<DescrptorAllocatorPage>>;

		std::set<size_t> m_availablePages;

		std::mutex allocationMutex;
	};
}
