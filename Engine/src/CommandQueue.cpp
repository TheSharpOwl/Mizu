#include "CommandQueue.hpp"

#include "Helpers.h"

#include<cassert>
//#include "..\inc\CommandQueue.hpp"


namespace Mizu
{
	template<typename T>
	using cp = Microsoft::WRL::ComPtr<T>;

	CommandQueue::CommandQueue(Microsoft::WRL::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type) :
		m_fenceValue(0),
		m_device(device),
		m_commandListType(type)
	{
		D3D12_COMMAND_QUEUE_DESC des = {};
		des.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		des.NodeMask = 0;
		des.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		des.Type = m_commandListType;

		ThrowIfFailed(m_device->CreateCommandQueue(&des, IID_PPV_ARGS(&m_commandQueue)));
		ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
		m_fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

		assert(m_fenceEvent && "Failed to create the fence event handle!");
	}

	CommandQueue::~CommandQueue()
	{

	}

	uint64_t CommandQueue::executeCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>  commandList)
	{
		commandList->Close();

		ID3D12CommandAllocator* commandAllocator;
		UINT dataSize = sizeof(commandAllocator);
		ThrowIfFailed(commandList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &commandAllocator));

		ID3D12CommandList* const ppCommandList[] = {
			commandList.Get()
		};

		m_commandQueue->ExecuteCommandLists(1, ppCommandList);
		uint64_t fenceValue = signal();

		m_commandAllocatorQueue.emplace(CommandAllocatorEntry{ fenceValue, commandAllocator });
		m_commandListQueue.push(commandList);


		// because ownership was moved to the ComPtr in the commandAllocatorQueue, so it's safe to release now
		commandAllocator->Release();

		return fenceValue;
	}

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CommandQueue::getCommandList()
	{
		cp<ID3D12GraphicsCommandList2> commandList;
		cp<ID3D12CommandAllocator> commandAllocator;

		// reuse the command allocator we have if it's already finished 
		if (!m_commandAllocatorQueue.empty() && isFenceComplete(m_commandAllocatorQueue.front().fenceValue))
		{
			commandAllocator = m_commandAllocatorQueue.front().commandAllocator;
			m_commandAllocatorQueue.pop();

			ThrowIfFailed(commandAllocator->Reset());
		}
		else // or create a new one
		{
			commandAllocator = createCommandAllocator();
		}


		if (!m_commandListQueue.empty())
		{
			commandList = m_commandListQueue.front();
			m_commandListQueue.pop();

			ThrowIfFailed(commandList->Reset(commandAllocator.Get(), nullptr));
		}
		else
		{
			commandList = createCommandList(commandAllocator);
		}

		// we associate the command allocator with the command list so that we can retrieve the command allocator from the command list when it is executed
		UINT data_size = sizeof(commandAllocator);
		ThrowIfFailed(commandList->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), commandAllocator.Get()));

		return commandList;
	}

	uint64_t CommandQueue::signal()
	{
		uint64_t newFenceVal = ++m_fenceValue;
		ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), newFenceVal));
		return newFenceVal;
	}

	bool CommandQueue::isFenceComplete(uint64_t fenceValue)
	{
		return m_fence->GetCompletedValue() >= fenceValue;
	}

	void CommandQueue::waitForFenceValue(uint64_t fenceValue)
	{
		if (!isFenceComplete(fenceValue))
		{
			m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
			::WaitForSingleObject(m_fenceEvent, DWORD_MAX);
		}
	}
	void CommandQueue::flush()
	{
		waitForFenceValue(signal());
	}

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue::getCommandQueue() const
	{
		return m_commandQueue;
	}


	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandQueue::createCommandAllocator()
	{
		cp<ID3D12CommandAllocator> commandAllocator;
		ThrowIfFailed(m_device->CreateCommandAllocator(m_commandListType, IID_PPV_ARGS(&commandAllocator)));

		return commandAllocator;
	}

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CommandQueue::createCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator)
	{
		cp<ID3D12GraphicsCommandList2> commandList;
		ThrowIfFailed(m_device->CreateCommandList(0, m_commandListType, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

		// no need to close it inside our class 
		//ThrowIfFailed(commandList->Close());
		return commandList;

	}
}