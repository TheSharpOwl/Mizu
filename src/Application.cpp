#include "DX12LibPCH.h"
#include "Application.hpp"
#include "CommandQueue.hpp"
#include "Window.hpp"
#include "Game.hpp"
#include <map>

template<typename T>
using sp = shared_ptr<T>;

template<typename T>
using cp = Microsoft::WRL::ComPtr<T>;

static std::map<sp<Window>, HWND> g_windowHWndMap;
static std::map<std::wstring, sp<Window>> g_NameWindowMap;

constexpr wchar_t window_class_name[] = L"Mizu Demo";

// TODO make it a smart pointer
static Application* AppSingelton = nullptr;


struct MakeWindow : public Window
{
	MakeWindow(HWND hWnd, const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync)
		: Window(hWnd, windowName, clientWidth, clientHeight, vSync)
	{}
};



Application::Application(HINSTANCE hInst) :
	m_hInstance(hInst),
	m_TearingSupported(false)
{
	// Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
	// Using this awareness context allows the client area of the window 
	// to achieve 100% scaling while still allowing non-client window content to 
	// be rendered in a DPI sensitive fashion.
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

#if defined(_DEBUG)
	cp<ID3D12Debug> debugInter;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInter));
	debugInter->EnableDebugLayer();
#endif

	WNDCLASSEXW wndClass = { 0 };

	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = &WndProc;
	wndClass.hInstance = m_hInstance;
	wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndClass.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(APP_ICON));
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndClass.lpszMenuName = nullptr;
	wndClass.lpszClassName = window_class_name;
	wndClass.hIconSm = LoadIcon(m_hInstance, MAKEINTRESOURCE(APP_ICON));

	if (!RegisterClassExW(&wndClass))
	{
		MessageBoxA(NULL, "Unable to register the window class.", "Error", MB_OK | MB_ICONERROR);
	}

	m_Adapter = GetAdapter(false);
	if (m_Adapter)
	{
		m_Device = CreateDevice(m_Adapter);
	}
	if (m_Device)
	{
		m_DirectCommandQueue = std::make_shared<CommandQueue>(m_Device, D3D12_COMMAND_LIST_TYPE_DIRECT);
		m_ComputeCommandQueue = std::make_shared<CommandQueue>(m_Device, D3D12_COMMAND_LIST_TYPE_COMPUTE);
		m_CopyCommandQueue = std::make_shared<CommandQueue>(m_Device, D3D12_COMMAND_LIST_TYPE_COPY);

		m_TearingSupported = CheckTearingSupport();
	}
}

Application::~Application() {}

void Application::Create(HINSTANCE hInst)
{
	if (!AppSingelton)
		AppSingelton = new Application(hInst);
	// TODO log that there's already and instance
}

void Application::Destroy()
{
	if (AppSingelton)
	{
		assert(g_NameWindowMap.empty() && g_windowHWndMap.empty() &&
			"All windows should be destroyed before destroying the application instance.");

		delete AppSingelton;
		AppSingelton = nullptr;
	}
}

Application& Application::Get()
{
	assert(AppSingelton);
	return *AppSingelton;
}

bool Application::IsTearingSupported() const
{
	return m_TearingSupported;
}

std::shared_ptr<Window> Application::CreateRenderWindow(const std::wstring& windowName, int width, int height, bool vsync /*= true*/)
{
	// First check if a window with the given name already exists.
	auto windowIter = g_NameWindowMap.find(windowName);
	if (windowIter != g_NameWindowMap.end())
	{
		return windowIter->second;
	}

	RECT windowRect = { 0, 0, width, height };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	HWND hWnd = CreateWindowW(window_class_name, windowName.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr, nullptr, m_hInstance, nullptr);

	if (!hWnd)
	{
		MessageBoxA(NULL, "Could not create the render window.", "Error", MB_OK | MB_ICONERROR);
		return nullptr;
	}

	auto pWindow = std::make_shared<MakeWindow>(hWnd, windowName, width, height, vsync);

	g_windowHWndMap[hWnd] = pWindow;
	g_NameWindowMap[windowName] = pWindow;

	return pWindow;
}

void Application::DestroyWindow(const std::wstring& windowName)
{
	auto window_ptr = GetWindowByName(windowName);
	DestroyWindow(window_ptr);
}

void Application::DestroyWindow(std::shared_ptr<Window> window_ptr)
{
	if (window_ptr) window_ptr->Destroy();
}

std::shared_ptr<Window> Application::GetWindowByName(const std::wstring& windowName)
{
	std::shared_ptr<Window> window_ptr;
	auto it = g_NameWindowMap.find(windowName);

	if (it != g_NameWindowMap.end())
		window_ptr = it->second;

	return window_ptr;

}

int Application::Run(std::shared_ptr<Game> gamePtr)
{
	if (!gamePtr->Initialize()) return 1;
	if (!gamePtr->LoadContent()) return 2;

	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	// Flush any commands in the commands queues before quiting.
	Flush();

	gamePtr->UnloadContent();
	gamePtr->Destroy();

	return static_cast<int>(msg.wParam);
}




void Application::Quit(int exitCode = 0) 
{
	PostQuitMessage(exitCode);
}


Microsoft::WRL::ComPtr<ID3D12Device2> Application::GetDevice() const 
{
	return m_Device;
}


std::shared_ptr<CommandQueue> Application::GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) const
{
	std::shared_ptr<CommandQueue> commandQueue;
	switch (type)
	{
	case D3D12_COMMAND_LIST_TYPE_DIRECT:
		commandQueue = m_DirectCommandQueue;
		break;
	case D3D12_COMMAND_LIST_TYPE_COMPUTE:
		commandQueue = m_ComputeCommandQueue;
		break;
	case D3D12_COMMAND_LIST_TYPE_COPY:
		commandQueue = m_CopyCommandQueue;
		break;
	default:
		assert(false && "Invalid command queue type.");
	}

	return commandQueue;
}

void Application::Flush()
{
	m_DirectCommandQueue->Flush();
	m_ComputeCommandQueue->Flush();
	m_CopyCommandQueue->Flush();
}


Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> Application::CreateDescriptorHeap(UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type) {}

UINT Application::GetDescriptorHandleIncreamentSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const {}


Microsoft::WRL::ComPtr<IDXGIAdapter4> Application::GetAdapter(bool useWarp) 
{

}
Microsoft::WRL::ComPtr<ID3D12Device2> Application::CreateDevice(Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter)
{
	cp<ID3D12Device2> device2;
	ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device2)));


#if defined(_DEBUG)
	cp<ID3D12InfoQueue> pInfoQueue;
	if (SUCCEEDED(device2.As(&pInfoQueue)))
	{
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

		// Suppress whole categories of messages
		//D3D12_MESSAGE_CATEGORY Categories[] = {};

		// Suppress messages based on their severity level
		D3D12_MESSAGE_SEVERITY Severities[] =
		{
			D3D12_MESSAGE_SEVERITY_INFO
		};

		// Suppress individual messages by their ID
		D3D12_MESSAGE_ID DenyIds[] = {
			D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
		};

		D3D12_INFO_QUEUE_FILTER NewFilter = {};
		//NewFilter.DenyList.NumCategories = _countof(Categories);
		//NewFilter.DenyList.pCategoryList = Categories;
		NewFilter.DenyList.NumSeverities = _countof(Severities);
		NewFilter.DenyList.pSeverityList = Severities;
		NewFilter.DenyList.NumIDs = _countof(DenyIds);
		NewFilter.DenyList.pIDList = DenyIds;

		ThrowIfFailed(pInfoQueue->PushStorageFilter(&NewFilter));
	}
#endif


	return device2;
}

bool Application::CheckTearingSupport() 
{
	BOOL allowTearing = FALSE;

	// Rather than create the DXGI 1.5 factory interface directly, we create the
	// DXGI 1.4 interface and query for the 1.5 interface. This is to enable the 
	// graphics debugging tools which will not support the 1.5 factory interface 
	// until a future update.
	cp<IDXGIFactory4> f4;
	if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&f4))))
	{
		cp<IDXGIFactory5> f5;
		if (SUCCEEDED(f4.As(&f5)))
		{
			f5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING,
				&allowTearing, sizeof(allowTearing));
		}
	}

	return allowTearing;
}


MouseButtonEventArgs::MouseButton DecodeMouseButton(UINT messageID)
{
	MouseButtonEventArgs::MouseButton mouseButton = MouseButtonEventArgs::None;
	switch (messageID)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
	{
		mouseButton = MouseButtonEventArgs::Left;
	}
	break;
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_RBUTTONDBLCLK:
	{
		mouseButton = MouseButtonEventArgs::Right;
	}
	break;
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MBUTTONDBLCLK:
	{
		mouseButton = MouseButtonEventArgs::Middel;
	}
	break;
	}

	return mouseButton;
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	std::shared_ptr<Window> pWindow;
	{
		auto iter = g_windowHWndMap.find(hwnd);
		if (iter != g_windowHWndMap.end())
		{
			pWindow = iter->second;
		}
	}

	if (pWindow)
	{
		switch (message)
		{
		case WM_PAINT:
		{
			// Delta time will be filled in by the Window.
			UpdateEventArgs updateEventArgs(0.0f, 0.0f);
			pWindow->OnUpdate(updateEventArgs);
			RenderEventArgs renderEventArgs(0.0f, 0.0f);
			// Delta time will be filled in by the Window.
			pWindow->OnRender(renderEventArgs);
		}
		break;
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			MSG charMsg;
			// Get the Unicode character (UTF-16)
			unsigned int c = 0;
			// For printable characters, the next message will be WM_CHAR.
			// This message contains the character code we need to send the KeyPressed event.
			// Inspired by the SDL 1.2 implementation.
			if (PeekMessage(&charMsg, hwnd, 0, 0, PM_NOREMOVE) && charMsg.message == WM_CHAR)
			{
				GetMessage(&charMsg, hwnd, 0, 0);
				c = static_cast<unsigned int>(charMsg.wParam);
			}
			bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
			bool control = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
			bool alt = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
			KeyCode::Key key = (KeyCode::Key)wParam;
			unsigned int scanCode = (lParam & 0x00FF0000) >> 16;
			KeyEventArgs keyEventArgs(key, c, KeyEventArgs::Pressed, shift, control, alt);
			pWindow->OnKeyPressed(keyEventArgs);
		}
		break;
		case WM_SYSKEYUP:
		case WM_KEYUP:
		{
			bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
			bool control = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
			bool alt = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
			KeyCode::Key key = (KeyCode::Key)wParam;
			unsigned int c = 0;
			unsigned int scanCode = (lParam & 0x00FF0000) >> 16;

			// Determine which key was released by converting the key code and the scan code
			// to a printable character (if possible).
			// Inspired by the SDL 1.2 implementation.
			unsigned char keyboardState[256];
			GetKeyboardState(keyboardState);
			wchar_t translatedCharacters[4];
			if (int result = ToUnicodeEx(static_cast<UINT>(wParam), scanCode, keyboardState, translatedCharacters, 4, 0, NULL) > 0)
			{
				c = translatedCharacters[0];
			}

			KeyEventArgs keyEventArgs(key, c, KeyEventArgs::Released, shift, control, alt);
			pWindow->OnKeyReleased(keyEventArgs);
		}
		break;
		// The default window procedure will play a system notification sound 
		// when pressing the Alt+Enter keyboard combination if this message is 
		// not handled.
		case WM_SYSCHAR:
			break;
		case WM_MOUSEMOVE:
		{
			bool lButton = (wParam & MK_LBUTTON) != 0;
			bool rButton = (wParam & MK_RBUTTON) != 0;
			bool mButton = (wParam & MK_MBUTTON) != 0;
			bool shift = (wParam & MK_SHIFT) != 0;
			bool control = (wParam & MK_CONTROL) != 0;

			int x = ((int)(short)LOWORD(lParam));
			int y = ((int)(short)HIWORD(lParam));

			MouseMotionEventArgs mouseMotionEventArgs(lButton, mButton, rButton, control, shift, x, y);
			pWindow->OnMouseMoved(mouseMotionEventArgs);
		}
		break;
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		{
			bool lButton = (wParam & MK_LBUTTON) != 0;
			bool rButton = (wParam & MK_RBUTTON) != 0;
			bool mButton = (wParam & MK_MBUTTON) != 0;
			bool shift = (wParam & MK_SHIFT) != 0;
			bool control = (wParam & MK_CONTROL) != 0;

			int x = ((int)(short)LOWORD(lParam));
			int y = ((int)(short)HIWORD(lParam));

			MouseButtonEventArgs mouseButtonEventArgs(DecodeMouseButton(message), MouseButtonEventArgs::Pressed, lButton, mButton, rButton, control, shift, x, y);
			pWindow->OnMouseButtonPressed(mouseButtonEventArgs);
		}
		break;
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		{
			bool lButton = (wParam & MK_LBUTTON) != 0;
			bool rButton = (wParam & MK_RBUTTON) != 0;
			bool mButton = (wParam & MK_MBUTTON) != 0;
			bool shift = (wParam & MK_SHIFT) != 0;
			bool control = (wParam & MK_CONTROL) != 0;

			int x = ((int)(short)LOWORD(lParam));
			int y = ((int)(short)HIWORD(lParam));

			MouseButtonEventArgs mouseButtonEventArgs(DecodeMouseButton(message), MouseButtonEventArgs::Released, lButton, mButton, rButton, control, shift, x, y);
			pWindow->OnMouseButtonReleased(mouseButtonEventArgs);
		}
		break;
		case WM_MOUSEWHEEL:
		{
			// The distance the mouse wheel is rotated.
			// A positive value indicates the wheel was rotated to the right.
			// A negative value indicates the wheel was rotated to the left.
			float zDelta = ((int)(short)HIWORD(wParam)) / (float)WHEEL_DELTA;
			short keyStates = (short)LOWORD(wParam);

			bool lButton = (keyStates & MK_LBUTTON) != 0;
			bool rButton = (keyStates & MK_RBUTTON) != 0;
			bool mButton = (keyStates & MK_MBUTTON) != 0;
			bool shift = (keyStates & MK_SHIFT) != 0;
			bool control = (keyStates & MK_CONTROL) != 0;

			int x = ((int)(short)LOWORD(lParam));
			int y = ((int)(short)HIWORD(lParam));

			// Convert the screen coordinates to client coordinates.
			POINT clientToScreenPoint;
			clientToScreenPoint.x = x;
			clientToScreenPoint.y = y;
			ScreenToClient(hwnd, &clientToScreenPoint);

			MouseWheelEventArgs mouseWheelEventArgs(zDelta, lButton, mButton, rButton, control, shift, (int)clientToScreenPoint.x, (int)clientToScreenPoint.y);
			pWindow->OnMouseWheel(mouseWheelEventArgs);
		}
		break;
		case WM_SIZE:
		{
			int width = ((int)(short)LOWORD(lParam));
			int height = ((int)(short)HIWORD(lParam));

			ResizeEventArgs resizeEventArgs(width, height);
			pWindow->OnResize(resizeEventArgs);
		}
		break;
		case WM_DESTROY:
		{
			// If a window is being destroyed, remove it from the 
			// window maps.
			RemoveWindow(hwnd);

			if (g_windowHWndMap.empty())
			{
				// If there are no more windows, quit the application.
				PostQuitMessage(0);
			}
		}
		break;
		default:
			return DefWindowProcW(hwnd, message, wParam, lParam);
		}
	}
	else
	{
		return DefWindowProcW(hwnd, message, wParam, lParam);
	}

	return 0;
}