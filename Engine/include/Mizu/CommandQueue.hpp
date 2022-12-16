#pragma once

#include <d3dx12.h>
#include <wrl.h>

#include <cstdint>
#include <queue>

namespace Mizu
{
class CommandQueue
{
public:
    CommandQueue(Microsoft::WRL::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);
    virtual ~CommandQueue();

    uint64_t executeCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> GetCommandList();

    uint64_t signal();
    bool isFenceComplete(uint64_t fenceValue);
    void waitForFenceValue(uint64_t fenceValue);
    void flush();

    // TODO find a better solution than this one to close the window handle
    void closeHandle() const { ::CloseHandle(m_FenceEvent); }

    [[nodiscard]] Microsoft::WRL::ComPtr<ID3D12CommandQueue> getCommandQueue() const;

protected:
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CreateCommandAllocator();

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CreateCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator);

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
}
