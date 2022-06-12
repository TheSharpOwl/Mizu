#include "UploadBuffer.hpp"

#include "Application.hpp"
#include "Utils.hpp"

// TODO add messages for bad alloc

namespace Mizu
{
	UploadBuffer::AllocationAddress Mizu::UploadBuffer::allocate(size_t sizeBytes, size_t align)
	{
		if(sizeBytes > m_pageSize)
		{
			throw std::bad_alloc();
		}

		if(!m_curPage || !m_curPage->hasSpace(sizeBytes, align))
		{
			m_curPage = requestPage();
		}

		return m_curPage->allocate(sizeBytes, align);
	}

	void UploadBuffer::reset()
	{
		m_curPage = nullptr;

		// put all pages in the recycled place
		m_availablePages = m_pagePool;

		// reset all of them to be sure
		for (auto p : m_availablePages)
			p->reset();
	}

	std::shared_ptr<UploadBuffer::Page> UploadBuffer::requestPage()
	{
		std::shared_ptr<Page> page;

		// try to use a recycled page
		if(!m_availablePages.empty())
		{
			page = m_availablePages.front();
			m_availablePages.pop_front();
		}
		else // create a new one and add it to the pool
		{
			page = std::make_shared<Page>(m_pageSize);
			m_pagePool.push_back(page);
		}

		return page;
	}

	UploadBuffer::Page::Page(size_t sizeBytes)
		: m_size(sizeBytes)
		, m_offset(0)
	{
		auto device = Application::Get().GetDevice();

		/*
		 * Create a heap where we can upload from cpu to gpu (D3D12_HEAP_TYPE_UPLOAD)
		 * The initial state for such type should be Initial read
		 */
		CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto buffer = CD3DX12_RESOURCE_DESC::Buffer(m_size);

		ThrowIfFailed(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &buffer, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_resource)));

		m_address.gpu = m_resource->GetGPUVirtualAddress();
		/* Map parameters:
		 * 1. index of the subresource
		 * 2. Range where cpu can read, nullptr means can read everything from it
		 * 3. pointer to cpu address
		 */
		m_resource->Map(0, nullptr, &m_address.cpu);
	}

	UploadBuffer::Page::~Page()
	{
		m_resource->Unmap(0, nullptr);
		m_address.cpu = nullptr;
		m_address.gpu = D3D12_GPU_VIRTUAL_ADDRESS(0);
	}

	bool UploadBuffer::Page::hasSpace(size_t sizeBytes, size_t align) const
	{
		// size of the whole thing starts after the offset (we offset then add the thing that's why everything is aligned alone then summed up)
		size_t alignedSize = Mizu::Math::AlignUp(sizeBytes, align);
		size_t alignedOffset = Mizu::Math::AlignUp(m_offset, align);

		return alignedOffset + alignedSize <= m_size;
	}
	
	UploadBuffer::AllocationAddress UploadBuffer::Page::allocate(size_t sizeBytes, size_t align)
	{
		if (!hasSpace(sizeBytes, align))
			throw std::bad_alloc();

		size_t alignedSize = Math::AlignUp(sizeBytes, align);
		m_offset = Math::AlignUp(sizeBytes, align);


		AllocationAddress address(static_cast<uint8_t*>(m_address.cpu) + m_offset, m_address.gpu + m_offset);

		m_offset += alignedSize;

		return address;
	}
}