#pragma once
#include <d3d12.h>

#include <cstdint>
#include <memory>

namespace Mizu
{

	class DescriptorAllocatorPage;

	class DescriptorAllocation
	{
	public:
		// Creates a NULL descriptor.
		DescriptorAllocation();

		DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, uint32_t numHandles, uint32_t descriptorSize, std::shared_ptr<DescriptorAllocatorPage> page);

		~DescriptorAllocation();

		// no copying
		DescriptorAllocation(const DescriptorAllocation&) = delete;
		DescriptorAllocation& operator=(const DescriptorAllocation&) = delete;

		// move is fine
		DescriptorAllocation(DescriptorAllocation&& allocation);
		DescriptorAllocation& operator=(DescriptorAllocation&& other);

		bool IsNull() const;

		// Get a descriptor at a particular offset in the allocation.
		D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle(uint32_t offset = 0) const;

		// Get the number of (consecutive) handles for this allocation.
		uint32_t GetNumHandles() const;

		// Get the heap that this allocation came from.
		// (For internal use only).
		std::shared_ptr<DescriptorAllocatorPage> GetDescriptorAllocatorPage() const;

	private:
		// Free the descriptor back to the heap it came from.
		void FreeDescriptor();

		// The base descriptor.
		D3D12_CPU_DESCRIPTOR_HANDLE m_descriptor;
		// The number of descriptors in this allocation.
		uint32_t m_numHandles;
		// The offset to the next descriptor.
		uint32_t m_descriptorSize;

		// A pointer back to the original page where this allocation came from.
		std::shared_ptr<DescriptorAllocatorPage> m_page;

	};
}