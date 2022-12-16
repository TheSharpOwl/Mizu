#include "CommandQueue.hpp"

#include "Helpers.h"

#include<cassert>
//#include "..\inc\CommandQueue.hpp"


namespace Mizu
{
	template<typename T>
	using cp = Microsoft::WRL::ComPtr<T>;

	CommandQueue::CommandQueue(Microsoft::WRL::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type) :
		m_FenceValue(0),
		m_Device(device),
		m_CommandListType(type)
	{
		D3D12_COMMAND_QUEUE_DESC des = {};
		des.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		des.NodeMask = 0;
		des.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		des.Type = m_CommandListType;

		ThrowIfFailed(m_Device->CreateCommandQueue(&des, IID_PPV_ARGS(&m_CommandQueue)));
		ThrowIfFailed(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));
		m_FenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

		assert(m_FenceEvent && "Failed to create the fence event handle!");
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

		m_CommandQueue->ExecuteCommandLists(1, ppCommandList);
		uint64_t fenceValue = signal();

		m_CommandAllocatorQueue.emplace(CommandAllocatorEntry{ fenceValue, commandAllocator });
		m_CommandListQueue.push(commandList);


		// because ownership was moved to the ComPtr in the commandAllocatorQueue, so it's safe to release now
		commandAllocator->Release();

		return fenceValue;
	}

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CommandQueue::GetCommandList()
	{
		cp<ID3D12GraphicsCommandList2> commandList;
		cp<ID3D12CommandAllocator> commandAllocator;

		// reuse the command allocator we have if it's already finished 
		if (!m_CommandAllocatorQueue.empty() && isFenceComplete(m_CommandAllocatorQueue.front().fenceValue))
		{
			commandAllocator = m_CommandAllocatorQueue.front().commandAllocator;
			m_CommandAllocatorQueue.pop();

			ThrowIfFailed(commandAllocator->Reset());
		}
		else // or create a new one
		{
			commandAllocator = CreateCommandAllocator();
		}


		if (!m_CommandListQueue.empty())
		{
			commandList = m_CommandListQueue.front();
			m_CommandListQueue.pop();

			ThrowIfFailed(commandList->Reset(commandAllocator.Get(), nullptr));
		}
		else
		{
			commandList = CreateCommandList(commandAllocator);
		}

		// we associate the command allocator with the command list so that we can retrieve the command allocator from the command list when it is executed
		UINT data_size = sizeof(commandAllocator);
		ThrowIfFailed(commandList->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), commandAllocator.Get()));

		return commandList;
	}

	uint64_t CommandQueue::signal()
	{
		uint64_t newFenceVal = ++m_FenceValue;
		ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), newFenceVal));
		return newFenceVal;
	}

	bool CommandQueue::isFenceComplete(uint64_t fenceValue)
	{
		return m_Fence->GetCompletedValue() >= fenceValue;
	}

	void CommandQueue::waitForFenceValue(uint64_t fenceValue)
	{
		if (!isFenceComplete(fenceValue))
		{
			m_Fence->SetEventOnCompletion(fenceValue, m_FenceEvent);
			::WaitForSingleObject(m_FenceEvent, DWORD_MAX);
		}
	}
	void CommandQueue::flush()
	{
		waitForFenceValue(signal());
	}

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue::getCommandQueue() const
	{
		return m_CommandQueue;
	}


	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandQueue::CreateCommandAllocator()
	{
		cp<ID3D12CommandAllocator> commandAllocator;
		ThrowIfFailed(m_Device->CreateCommandAllocator(m_CommandListType, IID_PPV_ARGS(&commandAllocator)));

		return commandAllocator;
	}

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CommandQueue::CreateCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator)
	{
		cp<ID3D12GraphicsCommandList2> commandList;
		ThrowIfFailed(m_Device->CreateCommandList(0, m_CommandListType, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

		// no need to close it inside our class 
		//ThrowIfFailed(commandList->Close());
		return commandList;

	}
}