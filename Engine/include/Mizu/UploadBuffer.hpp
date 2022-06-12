#pragma once

#include "Utils.hpp"

#include <DX12LibPCH.h>
#include "d3d12.h"
#include <wrl.h>

#include <deque>


namespace Mizu
{
	class UploadBuffer
	{
	public:
		struct AllocationAddress
		{
			AllocationAddress() : cpu(nullptr), gpu(D3D12_GPU_VIRTUAL_ADDRESS(0)) {}
			AllocationAddress(void* pCpu, D3D12_GPU_VIRTUAL_ADDRESS pGpu) : cpu(pCpu), gpu(pGpu) {}

			void* cpu;
			D3D12_GPU_VIRTUAL_ADDRESS gpu;
		};

		explicit UploadBuffer(size_t pageSize = mb(2)) : m_pageSize(pageSize) {}

		virtual ~UploadBuffer() = default;

		[[nodiscard]] size_t getPageSize() const { return m_pageSize; };


		AllocationAddress allocate(size_t sizeBytes, size_t align);


		void reset();


	protected:

		struct Page
		{
			Page(size_t sizeBytes);
			virtual ~Page();

			[[nodiscard]] bool hasSpace(size_t sizeBytes, size_t align) const;

			[[nodiscard]] AllocationAddress allocate(size_t sizeBytes, size_t align);

			void reset();

		protected:
			Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;

			AllocationAddress m_address;

			size_t m_size;
			size_t m_offset;
		};

		std::shared_ptr<Page> requestPage();

		using PagePool = std::deque<std::shared_ptr<Page>>;

		PagePool m_pagePool;
		// empty initially and filled with pages cleaned after being used (recycled pages) because it's faster to use recycle than request new pages or return every time
		PagePool m_availablePages;

		std::shared_ptr<Page> m_curPage;

		size_t m_pageSize;

	};
};