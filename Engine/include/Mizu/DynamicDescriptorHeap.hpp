#pragma once

#include "d3d12.h"
#include <cstdint>
#include <functional>

namespace Mizu
{
	class CommandList;
	class RootSignature;

	class DynamicDescriptorHeap
	{
	public:
		DynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, uint32_t descriptorsPerHeap = 1024);

		virtual ~DynamicDescriptorHeap();

		/**
		 * \brief Stages a contiguous range of CPU visible descriptors while they are not copied to the GPU visible descriptor heap until commitStagedDescriptors function is called
		 * \param rootParameterIdx index of the root parameter to copy the descriptors to (the index points to a table)
		 * \param offset the offset with the descriptor table to copy the descriptors to (the offset is inside the table the index is pointing to)
		 * \param descriptorsCount
		 * \param srcDescriptors base descriptor to start copying descriptors from
		 */
		void stageDescriptors(uint32_t rootParameterIdx, uint32_t offset, uint32_t descriptorsCount, const D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptors) {}


		/**
		 * \brief Copies all of the staged descriptors to GPU's visible descriptor heap and bindsthe desciptors heap and descriptor tables to the command list.
		 * \param commandList 
		 * \param setFunc Used to set the GPU visible descriptors on the command list, two possible possible functions are :
		 * 1. SetGraphicsRootDescriptorTable before a draw
		 * 2. SetComputeRootDescriptorTable	 before a dispatch
		 *
		 * or it might not know which function to use so it will be passed as an arg // TODO improve this documentation
		 */
		void commitStagedDescriptors(CommandList& commandList, std::function<void(ID3D12GraphicsCommandList*, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)> setFunc);
		void commitStagedDescriptorsForDraw(CommandList& commandList);
		void commitStagedDescriptorsForDispatch(CommandList& commandList);


	};
}
