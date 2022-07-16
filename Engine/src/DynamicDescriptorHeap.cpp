#include "DynamicDescriptorHeap.hpp"
#include "Application.hpp"
#include "CommandList.hpp"
#include "RootSignature.hpp"

#include "DX12LibPCH.h"

namespace Mizu
{
	DynamicDescriptorHeap::DynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, uint32_t descriptorsPerHeap)
		: m_descriptorHeapType(heapType)
		, m_descriptorsPerHeapCount(descriptorsPerHeap)
		, m_descriptorTableBitmask(0)
		, m_staleDescriptorTableBitmask(0)
		, m_freeHandlesCount(0)
		, m_currentGpuDescriptorHandle(D3D12_DEFAULT)
		, m_currentCpuDescriptorHandle(D3D12_DEFAULT)
	{
		m_descriptorHandleIncrementSize = Application::Get().GetDescriptorHandleIncrementSize(heapType);

		// allocate space to stage CPU visible descriptors
		m_descriptorHandleCache = std::make_unique<D3D12_CPU_DESCRIPTOR_HANDLE[]>(m_descriptorsPerHeapCount);
	}
	DynamicDescriptorHeap::~DynamicDescriptorHeap()
	{

	};

	void DynamicDescriptorHeap::commitStagedDescriptors(CommandList& commandList, std::function<void(ID3D12GraphicsCommandList*, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)> setFunc)
	{

	}

	void DynamicDescriptorHeap::commitStagedDescriptorsForDraw(CommandList& commandList)
	{

	}

	void DynamicDescriptorHeap::commitStagedDescriptorsForDispatch(CommandList& commandList)
	{

	}

	D3D12_GPU_DESCRIPTOR_HANDLE DynamicDescriptorHeap::copyDescriptor(CommandList& commandList, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor)
	{
		return D3D12_GPU_DESCRIPTOR_HANDLE{ 0 };
	}

	void DynamicDescriptorHeap::parseRootSignature(const RootSignature& rootSignature)
	{
		// when root signature changes all the descriptors have to be rebound to the command list
		m_staleDescriptorTableBitmask = 0;

		const auto& rootSignatureDesc = rootSignature.getRootSignatureDesc();
	}

	void DynamicDescriptorHeap::reset()
	{

	}
}