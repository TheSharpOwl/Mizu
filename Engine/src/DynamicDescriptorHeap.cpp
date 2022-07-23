#include "DynamicDescriptorHeap.hpp"

#include <stdexcept>

#include "Application.hpp"
#include "CommandList.hpp"
#include "RootSignature.hpp"

#include "DX12LibPCH.h"

template<typename T>
using cp = Microsoft::WRL::ComPtr<T>;

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
		uint32_t numDescriptorsToCommit = computeStaleDesctiptorCount();

		if (numDescriptorsToCommit == 0)
			return;

		auto device = Application::Get().GetDevice();
		auto cl = commandList.getGraphicsCommandList().Get();

		assert(cl != nullptr);

		if(!m_currentDescriptorHeap || m_freeHandlesCount < numDescriptorsToCommit)
		{
			m_currentDescriptorHeap = requestDescriptorHeap();
			m_currentCpuDescriptorHandle = m_currentDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			m_currentGpuDescriptorHandle = m_currentDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

			m_freeHandlesCount = m_descriptorsPerHeapCount;

			commandList.setDescriptorHeap(m_descriptorHeapType, m_currentDescriptorHeap.Get());

			// when we update the descriptor heap for the command list, all the descriptor tables must be (re)copied to the new descriptor not only the stale descriptors
			m_staleDescriptorTableBitmask = m_descriptorTableBitmask;
		}

		DWORD rootIndex;

		while(_BitScanForward(&rootIndex, m_staleDescriptorTableBitmask))
		{
			UINT numSrcDescriptors = m_descriptorTableCache[rootIndex].descriptorsCount;

			D3D12_CPU_DESCRIPTOR_HANDLE* pSrcDescriptorHandles = m_descriptorTableCache[rootIndex].baseDescriptor;
			D3D12_CPU_DESCRIPTOR_HANDLE pDestDescriptorRangeStarts[] =
			{
				m_currentCpuDescriptorHandle
			};

			UINT pDescriptorRangeSizes[] =
			{
				numSrcDescriptors
			};
			// copy from a cpu descriptor (non gpu visible) to another cpu one but visible to the gpu
			device->CopyDescriptors(1, pDestDescriptorRangeStarts, pDescriptorRangeSizes,
				numSrcDescriptors, pSrcDescriptorHandles, pDescriptorRangeSizes, m_descriptorHeapType);

			setFunc(cl, rootIndex, m_currentGpuDescriptorHandle);

			// offset the cpu and gpu handles
			m_currentGpuDescriptorHandle.Offset(numSrcDescriptors, m_descriptorHandleIncrementSize);
			m_currentCpuDescriptorHandle.Offset(numSrcDescriptors, m_descriptorHandleIncrementSize);
			
			m_freeHandlesCount -= numSrcDescriptors;

			// mark the rootIndex as complete
			m_staleDescriptorTableBitmask ^= (1 << rootIndex);
		}
	}

	void DynamicDescriptorHeap::commitStagedDescriptorsForDraw(CommandList& commandList)
	{
		commitStagedDescriptors(commandList, &ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable);
	}

	void DynamicDescriptorHeap::commitStagedDescriptorsForDispatch(CommandList& commandList)
	{
		commitStagedDescriptors(commandList, &ID3D12GraphicsCommandList::SetComputeRootDescriptorTable);
	}

	D3D12_GPU_DESCRIPTOR_HANDLE DynamicDescriptorHeap::copyDescriptor(CommandList& commandList, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor)
	{
		if(!m_currentDescriptorHeap || m_freeHandlesCount < 1)
		{
			m_currentDescriptorHeap = requestDescriptorHeap();
			m_currentCpuDescriptorHandle = m_currentDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			m_currentGpuDescriptorHandle = m_currentDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
			m_freeHandlesCount = m_descriptorsPerHeapCount;

			commandList.setDescriptorHeap(m_descriptorHeapType, m_currentDescriptorHeap.Get());

			// because when we updated the descriptor heap, every descriptor table must be recopied to the new descriptor heap no only the stale ones
			m_staleDescriptorTableBitmask = m_descriptorTableBitmask;
			// (it will be done before a draw of dispatch call)
		}

		auto device = Application::Get().GetDevice();
		D3D12_GPU_DESCRIPTOR_HANDLE hGpu = m_currentGpuDescriptorHandle; // we need to return it before we offset it
		device->CopyDescriptorsSimple(1, m_currentCpuDescriptorHandle, cpuDescriptor, m_descriptorHeapType);

		// only 1
		m_currentCpuDescriptorHandle.Offset(1, m_descriptorHandleIncrementSize);
		m_currentGpuDescriptorHandle.Offset(1, m_descriptorHandleIncrementSize);
		m_freeHandlesCount--;

		return hGpu;
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


	uint32_t DynamicDescriptorHeap::computeStaleDesctiptorCount() const
	{
		uint32_t result = 0;
		DWORD i;
		DWORD staleDescriptorsBitmask = m_staleDescriptorTableBitmask;

		while(_BitScanForward(&i, staleDescriptorsBitmask))
		{
			result += m_descriptorTableCache[i].descriptorsCount;
			staleDescriptorsBitmask ^= (1 << i);
		}

		return result;
	}

	void DynamicDescriptorHeap::reset()
	{
		m_availableDescriptorHeaps = m_descriptorHeapPool;
		m_currentDescriptorHeap.Reset();
		m_currentCpuDescriptorHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);
		m_currentGpuDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);
		m_freeHandlesCount = 0;
		m_descriptorTableBitmask = 0;
		m_staleDescriptorTableBitmask = 0;

		// reset the table cache
		for(int i = 0;i < MaxDescriptorTables;i++)
		{
			m_descriptorTableCache[i].reset();
		}
	}


	cp<ID3D12DescriptorHeap> DynamicDescriptorHeap::requestDescriptorHeap()
	{
		cp<ID3D12DescriptorHeap> descriptorHeap;
		if(!m_availableDescriptorHeaps.empty())
		{
			descriptorHeap = m_availableDescriptorHeaps.front();
			m_availableDescriptorHeaps.pop();
		}
		else
		{
			descriptorHeap = createDescriptorHeap();
			m_descriptorHeapPool.push(descriptorHeap);
		}

		return descriptorHeap;
	}

	cp<ID3D12DescriptorHeap> DynamicDescriptorHeap::createDescriptorHeap()
	{
		auto device = Application::Get().GetDevice();

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = m_descriptorsPerHeapCount;
		desc.Type = m_descriptorHeapType;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		cp<ID3D12DescriptorHeap> descriptorHeap;
		ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));

		return descriptorHeap;
	}


}