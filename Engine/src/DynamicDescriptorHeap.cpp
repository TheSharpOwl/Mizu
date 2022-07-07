#include "DynamicDescriptorHeap.hpp"
#include "RootSignature.hpp"

namespace Mizu
{
	DynamicDescriptorHeap::DynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, uint32_t descriptorsPerHeap)
	{

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

	}

	void DynamicDescriptorHeap::reset()
	{

	}
}