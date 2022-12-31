#pragma once

#include "DX12LibPCH.h"
#include <memory>
#include <utility>

#include <unordered_map>

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

namespace Mizu
{
    class Window;
    class CommandQueue;
    class Game;

    // TODO refactor function names to use lowerCamelCase not UpperCamelCase
    class Application
    {
    public:
        Application() = delete;

        Application(const Application& copy) = delete;
        Application& operator=(const Application& other) = delete;

        static void create(HINSTANCE hInst);

        std::shared_ptr<Window> createRenderWindow(const std::wstring& windowName, int width, int height);

        int run(std::shared_ptr<Game> game);

        static void destroy();

        void destroyWindow(const std::wstring& name);

        void destroyWindow(std::shared_ptr<Window> window);

        static Application& get();

        [[nodiscard]] bool checkTearingSupport() const;

        void flush();

        void close();

        [[nodiscard]] Microsoft::WRL::ComPtr<ID3D12Device2> getDevice() const;
        [[nodiscard]] std::shared_ptr<CommandQueue> getCommandQueue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT) const;
        [[nodiscard]] Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> getCommandList(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);

        // return the created descriptor heap with its size
        [[nodiscard]] std::pair<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>, UINT> createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors);

        [[nodiscard]] UINT getDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type);

        [[nodiscard]] auto getWindow(const std::wstring name) -> std::shared_ptr<Window>
        {
            auto it = ms_windowsNameMap.find(name);
            return (it != ms_windowsNameMap.end() ? it->second : nullptr);
        }
        [[nodiscard]] auto getWindow(HWND hWnd) -> std::shared_ptr<Window>
        {
            auto it = ms_windowsHwndMap.find(hWnd);
            return (it != ms_windowsHwndMap.end() ? it->second : nullptr);
        }

        /**
         * \brief remove window from our windows lists (the 2 maps HWND to window ptr and window name to its ptr)
         * \param hWnd hWnd of the window we want to remove
         */
        static void removeWindow(HWND hWnd);

        static uint64_t getFrameNumber();

        static const wchar_t* windowClassName;

    protected:
        Application(HINSTANCE hInst);

        Microsoft::WRL::ComPtr<IDXGIAdapter4> getAdapter();

        Microsoft::WRL::ComPtr<ID3D12Device2> createDevice();

        HINSTANCE m_hInstance;

        Microsoft::WRL::ComPtr<IDXGIAdapter4> m_adapter;
        Microsoft::WRL::ComPtr<ID3D12Device2> m_device;

        std::shared_ptr<CommandQueue> m_directCommandQueue;
        std::shared_ptr<CommandQueue> m_copyCommandQueue;
        std::shared_ptr<CommandQueue> m_computeCommandQueue;

        static std::unordered_map<std::wstring, std::shared_ptr<Window>> ms_windowsNameMap;
        static std::unordered_map<HWND, std::shared_ptr<Window>> ms_windowsHwndMap;

        bool m_isTearingSupported;

        const bool m_useWarp = false;

        static uint64_t ms_frameNumber;

        friend LRESULT CALLBACK::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    };
}