#include "DynamicDescriptorHeap.hpp"

#include <stdexcept>

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
		// vendor specific so queried in runtime
		m_descriptorHandleIncrementSize = Application::Get().GetDescriptorHandleIncrementSize(heapType);

		// allocate space to stage CPU visible descriptors
		m_descriptorHandleCache = std::make_unique<D3D12_CPU_DESCRIPTOR_HANDLE[]>(m_descriptorsPerHeapCount);
	}
	DynamicDescriptorHeap::~DynamicDescriptorHeap()
	{

	};

	void DynamicDescriptorHeap::stageDescriptors(uint32_t rootParameterIdx, uint32_t offset, uint32_t descriptorsCount, const D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptors)
	{
		// the number of descriptors should be less or equal to the number of descriptors per heap and the root index should be less than the max number of root params in the table
		if(descriptorsCount > m_descriptorsPerHeapCount || rootParameterIdx >= MaxDescriptorTables)
		{
			throw std::bad_alloc();
		}

		auto& descriptorTableCache = m_descriptorTableCache[rootParameterIdx];

		if(offset + descriptorsCount > descriptorTableCache.descriptorsCount)
		{
			throw std::length_error("Number of descriptors exceeds the number of descriptors in the descriptors table.");
		}

		D3D12_CPU_DESCRIPTOR_HANDLE* dstDescriptor = (descriptorTableCache.baseDescriptor + offset);
		for (uint32_t i = 0; i < descriptorsCount; ++i)
		{
			dstDescriptor[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(srcDescriptors, i, m_descriptorHandleIncrementSize);
		}

		m_staleDescriptorTableBitmask |= (1 << rootParameterIdx);
	}

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

		/*
		 * Whenever the root signature changes on the command list, any stale descriptors that were staged but not committed should be bound again to the graphics or compute pipelines. 
		 */
		const auto& rootSignatureDesc = rootSignature.getRootSignatureDesc();

		// get bitmask which represents the root parameter indices that match the descriptor heap type for this dynamic heap
		m_descriptorTableBitmask = rootSignature.getDescriptorTableBitMask(m_descriptorHeapType);
		uint32_t descriptorTableBitmask = m_descriptorTableBitmask;

		uint32_t currentOffset = 0;
		DWORD rootIndex;

		while(_BitScanForward(&rootIndex, descriptorTableBitmask) && rootIndex < rootSignatureDesc.NumParameters)
		{
			uint32_t numDescriptors = rootSignature.getDescriptorsCount(rootIndex);
			DesctiptorTableCache& descriptorTableCache = m_descriptorTableCache[rootIndex];
			descriptorTableCache.descriptorsCount = numDescriptors;
			descriptorTableCache.baseDescriptor = m_descriptorHandleCache.get() + currentOffset;

			currentOffset += numDescriptors;

			// flip the descriptor table bit to not scan it again
			descriptorTableBitmask ^= (1 << rootIndex);
		}

		assert(currentOffset <= m_descriptorsPerHeapCount && "The root signature"
		"requires more than the maximum number of descriptors per descriptor heap. Consider increasing the maximum number of descriptors per descriptor heap.");
	}

	void DynamicDescriptorHeap::reset()
	{

	}
}