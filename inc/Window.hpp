#pragma once

#define WIN32_LEAN_AND_MEAN

#include<string>

#include<Windows.h>

#include<wrl.h>
#include<d3d12.h>
#include<dxgi1_5.h>

#include "Helpers.h"

class Window
{
public:

	static const unsigned int BufferCount = 2U;


	HWND GetWindowHandle() const;


	void Destroy();


	const std::wstring& GetWindowName() const;

	int GetClientHeight;
	int GetClientWidth;

	bool IsVSync() const;
	void SetVSync(bool vSync);
	//void ToggleVSync();


	bool IsFullScreen() const;
	void SetFullScreen(bool fullscreen);
	void ToggleFullScreen();


	void Show();


	void Hide();

	UINT GetCurrentBackBufferIndex() const;

	UINT Present();

	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentTargetRenderView() const;

	Microsoft::WRL::ComPtr<ID3D12Resource> GetCurrentBackBuffer() const;

	virtual ~Window();

protected:

	Microsoft::WRL:: ComPtr<IDXGISwapChain4> CreateSwapChain();

	friend LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	friend class Application;

	friend class Game;


	Window(HWND hWnd, const std::wstring& windowName, int width, int height, bool vsync);


	virtual void OnUpdate(UpdateEventArgs& e);
	virtual void OnRender(RenderEventArgs& e);

	virtual void OnKeyPressed(KeyEventArgs& e);
	virtual void OnKeyReleased(KeyEventArgs& e);
	virtual void OnMouseMoved(MouseMotionEventArgs& e);
	virtual void OnMouseButtonPressed(MouseButtonEventArgs& e);
	virtual void OnMouseButtonReleased(MouseButtonEventArgs& e);
	virtual void OnMouseWheel(MouseWheelEventArgs& e);
	virtual void OnResize(ResizeEventArgs& e);

	Microsoft::WRL::ComPtr<IDXGISwapChain4> CreateSwapChain();

	void UpdateRenderTargetViews();

private:
	
	HWND m_hWnd;
	std::wstring m_WindowName;

	int m_ClientWidth;
	int m_ClientHeight;
	bool m_VSync;
	bool m_FullScreen;


	Microsoft::WRL::ComPtr<IDXGISwapChain4> m_SwapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RTVDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_BackBuffers[BufferCount];

	UINT m_RTVDescriptorSize;
	UINT m_CurrentBackBufferIndex;

	RECT m_WindowRect;
	bool m_TearingSupported;
};

