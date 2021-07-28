#pragma once
#include "DX12LibPCH.h"
#include <memory>

class CommandQueue;

// TODO add Renderer class and improve dependencies (for example app and window)

namespace Mizu
{

	class Window
	{
	public:

		Window(const wchar_t* windowClassName, const wchar_t* windowTitle, HINSTANCE hInst, uint32_t windowWidth, uint32_t windowHeight);

		static const uint32_t numberOfBuffers = 2;

		static bool CheckTearingSupport();

		UINT GetCurrentBackBufferIndex() const;

		void Present(const UINT syncInterval, const UINT presentFlags) const;

		void Resize(uint32_t newWidth, uint32_t newHeight);

		void Update();

		void Render();

	protected:

		friend LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		void RegisterWindowClass(HINSTANCE hInstance, const wchar_t* windowClassName);

		Microsoft::WRL::ComPtr<IDXGISwapChain4> CreateSwapChain(std::shared_ptr<CommandQueue> command_queue_sptr);

		HWND m_hWnd;

		uint32_t  m_screenWidth;
		uint32_t  m_screenHeight;

		uint32_t m_windowWidth;
		uint32_t m_windowHeight;

		RECT m_windowRect;

		Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swapChain;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_backBuffers[numberOfBuffers];

		UINT m_currentBackBufferIndex;

		uint64_t m_frameFenceValues[numberOfBuffers];

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RTVDescriptorHeap;
		UINT m_RTVDescriptorSize;
	};
}			