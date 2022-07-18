#pragma once

#include "d3d12.h"
#include <cstdint>
#include <d3dx12.h>
#include <functional>
#include <queue>
#include <wrl/client.h>

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
		void stageDescriptors(uint32_t rootParameterIdx, uint32_t offset, uint32_t descriptorsCount, const D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptors)


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

		// TODO I did not understand this function well
		/**
		 * \brief Copies a single CPU descriptor to a GPU visible Descriptor heap which is useful ClearUnorderedAccessViewFloat and ClearUnorderedAccessViewUint
		 * methods because they require both a CPU and GPU visible descriptor for a UAV resource
		 * \param commandList in case the GPU visible descriptor needs to be updated on the command list
		 * \param cpuDescriptor the CPU descriptor to copy into a GPU visible descriptor
		 * \return the GPU visible descriptor
		 */
		D3D12_GPU_DESCRIPTOR_HANDLE copyDescriptor(CommandList& commandList, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor);

		/**
		 * \brief Parses a root signature to know which root parameters contain descriptor tables and determine number of descriptors needed for each table
		 * \param rootSignature
		 */
		void parseRootSignature(const RootSignature& rootSignature);

		/**
		 * \brief Resets the used descriptors and should by only used when all the descriptors that are referenced by a command list have finished execution on the command queue
		 */
		void reset();


	private:

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> requestDescriptorHeap();

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> createDescriptorHeap();

		uint32_t computeStaleDesctiptorCount() const;


		static const uint32_t MaxDescriptorTables = 32;

		struct DesctiptorTableCache
		{
			DesctiptorTableCache() :
				descriptorsCount(0),
				baseDescriptor(nullptr)
			{}


			void reset()
			{
				descriptorsCount = 0;
				baseDescriptor = nullptr;
			}
			uint32_t descriptorsCount;

			D3D12_CPU_DESCRIPTOR_HANDLE* baseDescriptor;
		};

		/**
		 * \brief type can be: D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV or D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER
		 *  // TODO did not get this :This parameter also determines the type of GPU visible descriptor heap to
			// create.
		 */
		D3D12_DESCRIPTOR_HEAP_TYPE m_descriptorHeapType;

		uint32_t m_descriptorsPerHeapCount;

		uint32_t m_descriptorHandleIncrementSize;

		std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE[]> m_descriptorHandleCache;

		DesctiptorTableCache m_descriptorTableCache[MaxDescriptorTables];

		// turned on bits represent an index in the root signature that has a table
		uint32_t m_descriptorTableBitmask;

		// turned on bits represent tables in the root signature that has changed since the last time of copying descriptors
		uint32_t m_staleDescriptorTableBitmask;

		using DescriptorHeapPool = std::queue<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>>;

		DescriptorHeapPool m_descriptorHeapPool;
		DescriptorHeapPool m_availableDescriptorHeaps;

		uint32_t m_freeHandlesCount;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_currentDescriptorHeap;
		CD3DX12_GPU_DESCRIPTOR_HANDLE m_currentGpuDescriptorHandle;
		CD3DX12_CPU_DESCRIPTOR_HANDLE m_currentCpuDescriptorHandle;
	};
}
