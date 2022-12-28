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

        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> getCommandList();

        uint64_t signal();
        bool isFenceComplete(uint64_t fenceValue);
        void waitForFenceValue(uint64_t fenceValue);
        void flush();

        // TODO find a better solution than this one to close the window handle
        void closeHandle() const { ::CloseHandle(m_fenceEvent); }

        [[nodiscard]] Microsoft::WRL::ComPtr<ID3D12CommandQueue> getCommandQueue() const;

    protected:
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> createCommandAllocator();

        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> createCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator);

        struct CommandAllocatorEntry
        {
            uint64_t fenceValue;
            Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
        };

        D3D12_COMMAND_LIST_TYPE m_commandListType;
        Microsoft::WRL::ComPtr<ID3D12Device2> m_device;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
        Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
        HANDLE m_fenceEvent;
        uint64_t m_fenceValue;

        std::queue<CommandAllocatorEntry> m_commandAllocatorQueue;
        std::queue<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>> m_commandListQueue;
    };
}
