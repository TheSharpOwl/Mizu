#include "..\include\Mizu\UploadBuffer.hpp"

#include "Application.hpp"

#include "Helpers.h"

using namespace Mizu;

UploadBuffer::UploadBuffer(size_t pageSize) : m_pageSize(pageSize) {}


UploadBuffer::Allocation UploadBuffer::Allocate(size_t sizeInBytes, size_t alignment)
{
	if (sizeInBytes > m_pageSize)
		throw std::bad_alloc();

	std::lock_guard<std::mutex> lock(m_mutex);

	if (!m_curPage || !m_curPage->hasSpace(sizeInBytes, alignment))
	{
		m_curPage = requestPage();
	}

	return m_curPage->Allocate(sizeInBytes, alignment);
}

void UploadBuffer::Reset()
{
	std::lock_guard<std::mutex> lock(m_mutex);

	m_curPage = nullptr;
	m_freePages = m_pagePool;

	for (auto p : m_freePages)
		p->Reset();

}

std::shared_ptr<UploadBuffer::Page> UploadBuffer::requestPage()
{
	std::lock_guard<std::mutex> lock(m_mutex);

	std::shared_ptr<Page> res;

	if (m_freePages.empty())
	{
		res = std::make_shared<Page>(m_pageSize);
		m_pagePool.push_back(res);
	}
	else
	{
		res = m_freePages.front();
		m_freePages.pop_front();
	}

	return res;
}

UploadBuffer::Page::Page(size_t sizeInBytes) :
	m_pageSize(sizeInBytes),
	m_offset(0)
{
	auto device = Application::Get().GetDevice();

	auto properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto desc = CD3DX12_RESOURCE_DESC::Buffer(m_pageSize);

	ThrowIfFailed(
		device->CreateCommittedResource(&properties, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_resource))
	);

	m_pGpu = m_resource->GetGPUVirtualAddress();
	m_resource->Map(0, nullptr, &m_pCpu);
}

UploadBuffer::Page::~Page()
{
	m_resource->Unmap(0, nullptr);
	m_pCpu = nullptr;
	m_pGpu = D3D12_GPU_VIRTUAL_ADDRESS(0);
}

bool UploadBuffer::Page::hasSpace(size_t sizeInBytes, size_t alignment) const
{
	size_t alignedSize = AlignUp(sizeInBytes, alignment);
	size_t alignedOffset = AlignUp(m_offset, alignment);

	return alignedOffset + alignedSize <= m_pageSize;
}

UploadBuffer::Allocation UploadBuffer::Page::Allocate(size_t sizeInBytes, size_t alignment)
{
	if (!hasSpace(sizeInBytes, alignment))
	{
		// Can't allocate space from page.
		throw std::bad_alloc();
	}

	size_t alignedSize = AlignUp(sizeInBytes, alignment);
	m_offset = AlignUp(m_offset, alignment);

	Allocation allocation;
	allocation.CPU = static_cast<uint8_t*>(m_pCpu) + m_offset;
	allocation.GPU = m_pGpu + m_offset;

	m_offset += alignedSize;

	return allocation;
}

void UploadBuffer::Page::Reset()
{
	m_offset = 0;
}
