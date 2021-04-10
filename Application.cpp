
#include <Windows.h>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include "Helpers.h"

uint32_t width = 1280, height= 720;
HWND hWnd;
// to toggle fullscreen state
RECT WindowRect;

const wchar_t* windowClassName = L"MizuWindowClass";

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CLOSE:
			::PostQuitMessage(69);
			break;
	}

	return ::DefWindowProc(hWnd, message, wParam, lParam);
}

void RegisterWindowClass(HINSTANCE hInstance, const wchar_t* windowClassName)
{
	WNDCLASSEX windowClass = {};
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = &WndProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = hInstance;
	windowClass.hIcon = nullptr; //::LoadIcon(hInstance, NULL);
	windowClass.hCursor = nullptr;//::LoadCursor(NULL, IDC_ARROW);
	windowClass.hbrBackground = nullptr;//(HBRUSH)(COLOR_WINDOW + 1);
	windowClass.lpszMenuName = nullptr;
	windowClass.lpszClassName = windowClassName;
	windowClass.hIconSm = nullptr;

	static HRESULT hl = ::RegisterClassExW(&windowClass);
	assert(SUCCEEDED(hl));

}

RECT g_WindowRect;

HWND CreateWindow(const wchar_t* windowClassName, const wchar_t* windowTitle, HINSTANCE hInst, uint32_t windowWidth, uint32_t windowHeight)
{
	int screenWidth = ::GetSystemMetrics(SM_CXBORDER);
	int screenHeight = ::GetSystemMetrics(SM_CYBORDER);

	RECT windowRect = { 0, 0, static_cast<LONG>(windowWidth), static_cast<LONG>(windowHeight) };
	::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	windowWidth = windowRect.right - windowRect.left;
	windowHeight = windowRect.bottom - windowRect.top;

	// putting the window in the center
	int windowX = std::max<int>(0, (screenWidth - windowWidth) / 2);
	int windowY = std::max<int>(0, (screenHeight - windowHeight) / 2);
	DWORD d2 = GetLastError();
	HWND hWnd = ::CreateWindowExW(NULL, windowClassName, L"Mizu", WS_OVERLAPPEDWINDOW, windowX, windowY, windowWidth, windowHeight, NULL, NULL, hInst, nullptr);
	DWORD d = GetLastError();
	assert(hWnd && "Failed To create a window");
	return hWnd;
}


int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
	const wchar_t* windowClassName = L"MizuWindowClass";
	RegisterWindowClass(hInstance, windowClassName);
	HWND hWnd = CreateWindow(windowClassName, L"Mizu", hInstance, 300, 300);
	::ShowWindow(hWnd, SW_SHOW);
	while (1)
	{
	}
	return 0;
}	