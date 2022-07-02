#include "DescriptorAllocation.hpp"
#include "DescriptorAllocatorPage.hpp"
#include "Application.hpp"


namespace Mizu
{
	DescriptorAllocation::DescriptorAllocation()
		: m_descriptor{ 0 }, m_handlesCount(0), m_descriptorSize(0), m_page(nullptr)
	{}


	DescriptorAllocation::DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE handle, uint32_t handlesCount, uint32_t descriptorSize, std::shared_ptr<DescriptorAllocatorPage> pDescriptorAllocatorPage)
	: m_descriptor(handle), m_handlesCount(handlesCount), m_descriptorSize(descriptorSize), m_page(std::move(pDescriptorAllocatorPage))
	{}

	DescriptorAllocation::~DescriptorAllocation()
	{
		freeAllocation();
	}

	DescriptorAllocation::DescriptorAllocation(DescriptorAllocation&& allocation)
	: m_descriptor(allocation.m_descriptor), m_handlesCount(allocation.m_handlesCount), m_descriptorSize(allocation.m_descriptorSize), m_page(std::move(allocation.m_page))
	{
	}

	DescriptorAllocation& DescriptorAllocation::operator=(DescriptorAllocation&& other)
	{
		freeAllocation();

		m_descriptor = other.m_descriptor;
		m_handlesCount = other.m_handlesCount;
		m_descriptorSize = other.m_descriptorSize;
		m_page = std::move(other.m_page);


		other.m_descriptor.ptr = 0;
		other.m_handlesCount = 0;
		other.m_descriptorSize = 0;

		return *this;
	}

	void DescriptorAllocation::freeAllocation()
	{
		if(!isNull() && m_page)
		{
			m_page->freeDescriptor(std::move(*this), Application::getFrameNumber());

		}
	}
	

}