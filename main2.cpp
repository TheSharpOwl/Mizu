#include "Application.hpp"
#include "Window.hpp"

bool g_IsInitialized = false;

std::shared_ptr<Mizu::Window> ourWindow;

// TODO FOR NEXT TIME seems this is not getting called (maybe it is not thinking this is the one I registered the window class with ?!!!!!!)
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (g_IsInitialized)
	{
		switch (message)
		{
		case WM_PAINT:
			ourWindow->Update();
			ourWindow->Render();
			break;

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			bool alt = (::GetAsyncKeyState(VK_MENU) & 0x8000) != 0;

			switch (wParam)
			{
			//case 'V':
			//	g_VSync = !g_VSync;
			//	break;
			case VK_ESCAPE:
				::PostQuitMessage(0);
				break;
			case VK_RETURN:
			//	if (alt)
			//	{
			//case VK_F11:
			//	//SetFullscreen(!g_IsFullscreen);
			//	}
				break;
			}
		}
		break;
		// this case is for not getting a windows annoying sound
		case WM_SYSCHAR:
			break;

		case WM_SIZE: // TODO test it
		{
			RECT clientRect = {};
			::GetClientRect(hWnd, &clientRect);

			int width = clientRect.right - clientRect.left;
			int height = clientRect.bottom - clientRect.top;

			ourWindow->Resize(width, height);
		}
		break;

		case WM_DESTROY:
			::PostQuitMessage(0);
			break;
		default:
			return ::DefWindowProcW(hWnd, message, wParam, lParam);
		}
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
	Mizu::Application& app = Mizu::Application::Get();
	
	ourWindow = std::make_shared<Mizu::Window>(L"MizuWindowClass", L"Mizu Demo", hInstance, app.Width, app.Height);

	g_IsInitialized = true;

	ourWindow->ShowWindow();

	MSG msg = {};
	// to print for example
	while (msg.message != WM_QUIT)
	{
		if (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	app.Flush();

	// TODO find a better way to handle that close handle
	app.Close();

	return 0;
}