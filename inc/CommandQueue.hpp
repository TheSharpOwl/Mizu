#pragma once

#include <d3dx12.h>
#include <wrl.h>

#include<queue>
#include<cstdint>



// TODO put inside a namespace Mizu !!!
class CommandQueue
{
public:
	CommandQueue(Microsoft::WRL::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);
	virtual ~CommandQueue();

	uint64_t ExecuteCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>  commandList);

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> GetCommandList();

	uint64_t Signal();
	bool IsFenceComplete(uint64_t fenceValue);
	void WaitForFenceValue(uint64_t fenceValue);
	void Flush();

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue() const;

// todo make private/protected
public:
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(); // was protected

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CreateCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator); // was protected


protected:

	struct CommandAllocatorEntry
	{
		uint64_t fenceValue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	};
	D3D12_COMMAND_LIST_TYPE m_CommandListType;
	Microsoft::WRL::ComPtr<ID3D12Device2> m_Device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
	Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
	HANDLE m_FenceEvent;
	uint64_t m_FenceValue;

	std::queue<CommandAllocatorEntry> m_CommandAllocatorQueue;
	std::queue<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>> m_CommandListQueue;
};
