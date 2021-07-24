#pragma once
#include "DX12LibPCH.h"

namespace Mizu
{
	class Window
	{
	public:

		Window(const wchar_t* windowClassName, const wchar_t* windowTitle, HINSTANCE hInst, uint32_t windowWidth, uint32_t windowHeight);

		const uint32_t numberOfBuffers = 2;

		static bool checkTearingSupport();

	protected:

		friend LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		void RegisterWindowClass(HINSTANCE hInstance, const wchar_t* windowClassName);

		Microsoft::WRL::ComPtr<IDXGISwapChain4> CreateSwapChain(Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue);

		HWND hWnd;

		uint32_t  screenWidth;
		uint32_t  screenHeight;

		uint32_t windowWidth;
		uint32_t windowHeight;

		RECT windowRect;
	};
}