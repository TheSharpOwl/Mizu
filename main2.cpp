#include "Application.hpp"


bool g_IsInitialized = false;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (g_IsInitialized)
	{
		//switch (message)
		//{
		//case WM_PAINT:
		//	Update();
		//	Render();
		//	break;

		//case WM_SYSKEYDOWN:
		//case WM_KEYDOWN:
		//{
		//	bool alt = (::GetAsyncKeyState(VK_MENU) & 0x8000) != 0;

		//	switch (wParam)
		//	{
		//	case 'V':
		//		g_VSync = !g_VSync;
		//		break;
		//	case VK_ESCAPE:
		//		::PostQuitMessage(0);
		//		break;
		//	case VK_RETURN:
		//		if (alt)
		//		{
		//	case VK_F11:
		//		SetFullscreen(!g_IsFullscreen);
		//		}
		//		break;
		//	}
		//}
		//break;
		//// this case is for not getting a windows annoying sound
		//case WM_SYSCHAR:
		//	break;

		//case WM_SIZE:
		//{
		//	RECT clientRect = {};
		//	::GetClientRect(g_hWnd, &clientRect);

		//	int width = clientRect.right - clientRect.left;
		//	int height = clientRect.bottom - clientRect.top;

		//	Resize(width, height);
		//}
		//break;

		//case WM_DESTROY:
		//	::PostQuitMessage(0);
		//	break;
		//default:
		//	return ::DefWindowProcW(hWnd, message, wParam, lParam);
		//}
	}
	else
	{
		return ::DefWindowProcW(hWnd, message, wParam, lParam);
	}

	return 0;

	//	switch (message)
//	{
//	case WM_CLOSE:
//		::PostQuitMessage(69);
//		break;
//	}
//	return ::DefWindowProc(hWnd, message, wParam, lParam);

}






int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{

	Mizu::Application::Create(hInstance);


	//g_IsInitialized = true;

	//::ShowWindow(g_hWnd, SW_SHOW);
	//MSG msg = {};
	//// to print for example
	//while (msg.message != WM_QUIT)
	//{
	//	if (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	//	{
	//		::TranslateMessage(&msg);
	//		::DispatchMessage(&msg);

	//	}
	//}

	//g_CommandQueue->Flush();

	//::CloseHandle(g_FenceEvent);

	return 0;
}