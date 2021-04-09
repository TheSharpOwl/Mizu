
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

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CLOSE:
			PostQuitMessage(1);
			break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
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
	windowClass.hIcon = ::LoadIcon(hInstance, NULL);
	windowClass.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	windowClass.lpszMenuName = nullptr;
	windowClass.lpszClassName = windowClassName;
	windowClass.hIconSm = nullptr;

	static HRESULT hl = ::RegisterClassExW(&windowClass);
	assert(SUCCEEDED(hl));
}



HWND CreateWindow(const wchar_t* windowClassName, const wchar_t* windowTitle, HINSTANCE hInst, uint32_t windowWidth, uint32_t windowHeight)
{
	int screenWidth = ::GetSystemMetrics(SM_CXBORDER);
	int screenHeight = ::GetSystemMetrics(SM_CYBORDER);

	// putting the window in the center
	int windowX = std::clamp<int>((screenWidth - windowWidth) / 2, 0, screenWidth);
	int windowY = std::clamp<int>((screenHeight - windowHeight) / 2, 0, screenHeight);

	HWND hWnd = ::CreateWindowExW(NULL, windowClassName, L"Mizu", WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, windowX, windowY, windowWidth, windowHeight, NULL, NULL, hInst, nullptr);

	assert(hWnd && "Failed To create a window");
	return hWnd;
}


int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
	const wchar_t* windowClassName = L"MizuWindowClass";
	RegisterWindowClass(hInstance, windowClassName);
	HWND hWnd = CreateWindow(windowClassName, L"Mizu", hInstance, 1280, 720);
	//ShowWindow( hWnd,SW_SHOW );
	return 0;
}	