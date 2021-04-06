
#include <Windows.h>
#include <cstdint>
#include <cassert>

uint32_t width = 1280, height= 720;
HWND hWnd;
// to toggle fullscreen state
RECT WindowRect;

const wchar_t* windowClassName = L"MizuWindowClass";

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO change
	return 0;
}

void RegisterWindowClass(HINSTANCE hInstance, const wchar_t* windowClassName)
{
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = &WndProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = hInstance;
	windowClass.hIcon = nullptr;
	windowClass.hCursor = nullptr;
	windowClass.hbrBackground = nullptr;
	windowClass.lpszMenuName = nullptr;
	windowClass.lpszClassName = windowClassName;

	static HRESULT hl = ::RegisterClassExW(&windowClass);
	assert(SUCCEEDED(hl));
}


/*
HWND CreateWindow()
{
	return HWND();
}
*/

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
	//wchar_t* windowClassName = L"MizuWindowClass";
	// TODO
	//HWND = ::CreateWindowExW(NULL, windowClassName, L"Mizu", WS_OVERLAPPEDWINDOW, )
	return 0;
}	