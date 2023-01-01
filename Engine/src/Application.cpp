#include "Application.hpp"

#include "DX12LibPCH.h"
#include "winuser.h"

#include "CommandQueue.hpp"
#include "Window.hpp"
#include "Game.hpp"

using namespace Microsoft::WRL;
using namespace std;

static Mizu::Application* App = nullptr;
static bool isReady = false;
std::unordered_map<std::wstring, std::shared_ptr<Mizu::Window>> Mizu::Application::ms_windowsNameMap;
std::unordered_map<HWND, std::shared_ptr<Mizu::Window>> Mizu::Application::ms_windowsHwndMap;
const wchar_t* Mizu::Application::windowClassName = L"MizuWindowClass";
uint64_t Mizu::Application::ms_frameNumber = 0;



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (isReady)
	{
		std::shared_ptr<Mizu::Window> window = Mizu::Application::get().getWindow(hWnd);
		assert(window && "there is no window with this hwnd");

		switch (message)
		{
		case WM_PAINT:
			++Mizu::Application::ms_frameNumber;

			window->Update();
			window->Render();
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

                        window->Resize(width, height);
                }
                break;

                case WM_DESTROY:
                        // remove the window from the list of our windows
                        Mizu::Application::removeWindow(hWnd);
		    // if we have no more windows left quit the application
                        if (Mizu::Application::ms_windowsHwndMap.empty())
                        {
                                ::PostQuitMessage(0);
                        }
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
}

namespace Mizu
{
	Application::Application(HINSTANCE hInst) :
		m_hInstance(hInst),
		m_isTearingSupported(false)
	{
		// Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
		// Using this awareness context allows the client area of the window 
		// to achieve 100% scaling while still allowing non-client window content to 
		// be rendered in a DPI sensitive fashion.
		SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

		// Enable debug layer in case of debugging
#if defined(_DEBUG)
		ComPtr<ID3D12Debug> debugInterface;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
		debugInterface->EnableDebugLayer();
#endif

		WNDCLASSEXW windowClass = { 0 };

		windowClass.cbSize = sizeof(WNDCLASSEX);
		windowClass.style = CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc = &WndProc;
		windowClass.hInstance = hInst;
		windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
		windowClass.hIcon = NULL;
		windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		windowClass.lpszMenuName = nullptr;
		windowClass.lpszClassName = windowClassName;
		windowClass.hIconSm = NULL;

		static HRESULT hl = ::RegisterClassExW(&windowClass);
		if (!hl)
		{
			MessageBoxA(NULL, "Unable to register the window class.", "Error", MB_OK | MB_ICONERROR);
		}
		// get the adapter
		m_adapter = getAdapter();
                assert(m_adapter && "Failed to get adapter");
		// make a device
		m_device = createDevice();
                assert(m_device && "Failed to create device");
		// create the command queues
		m_directCommandQueue = make_shared<CommandQueue>(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
		m_copyCommandQueue = make_shared<CommandQueue>(m_device, D3D12_COMMAND_LIST_TYPE_COPY);
		m_computeCommandQueue = make_shared<CommandQueue>(m_device, D3D12_COMMAND_LIST_TYPE_COMPUTE);

	        m_isTearingSupported = checkTearingSupport();

	}
	void Application::create(HINSTANCE hInst) // static 
	{
		// make sure we have an app singleton already
		if (!App)
		{
			App = new Application(hInst);
		}
	}

	std::shared_ptr<Window> Mizu::Application::createRenderWindow(const std::wstring& windowName, int width, int height)
	{

		// create the window
		// TODO maybe we can return the existing one ?
		assert(ms_windowsNameMap.find(windowName) == ms_windowsNameMap.end() && "A window with the same name already exists");

		auto window = make_shared<Window>(windowClassName, windowName.c_str(), m_hInstance, width, height);
		ms_windowsNameMap[windowName] = window;
		ms_windowsHwndMap[window->getHWnd()] = window;

		isReady = true;

		return window;
	}

	int Mizu::Application::run(std::shared_ptr<Game> game)
	{
		if (!game->Initialize()) return 1;
		if (!game->LoadContent()) return 2;

		// to print for example
		BOOL bRet;
		MSG msg;
		while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
		{
			if (bRet == -1)
			{
				OutputDebugStringW(L"ERROR\n");
				break;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		// TODO notify game about window destruction
		game->UnloadContent();
		destroyWindow(game->getWindow()->getName());

		flush();

		return static_cast<int>(msg.wParam);
	}

	void Application::destroy() // static
        {
                if (App)
                {
                        assert(ms_windowsHwndMap.empty() && ms_windowsNameMap.empty() && "All windows should be destoryed before destroying the application instance");
                        delete App;
                        App = nullptr;
                }
		// TODO add notification to game + window destruction
	}

	void Mizu::Application::destroyWindow(const std::wstring& name)
	{
		auto pWindow= getWindow(name);
                pWindow->destroy();
	}

        void Application::destroyWindow(std::shared_ptr<Window> window)
	{
                window->destroy();
	}

        Application& Application::get() // static
	{
                assert(App);
		return *App;
	}

	bool Application::checkTearingSupport() const
        {
                BOOL allowTearing = FALSE;
                // it's made factory 4 then checked to be 5 for graphics debugging tools support (don't know if by this time they support it because in the tutorial it was not)
                ComPtr<IDXGIFactory4> factory4;
                if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
                {
                        ComPtr<IDXGIFactory5> factory5;
                        if (SUCCEEDED(factory4.As(&factory5)))
                        {
                                factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
                        }
                }

                return allowTearing == TRUE;
        }

	ComPtr<ID3D12Device2> Application::getDevice() const
	{
		return m_device;
	}

	shared_ptr<CommandQueue> Application::getCommandQueue(D3D12_COMMAND_LIST_TYPE type) const // type = D3D12_COMMAND_LIST_TYPE_DIRECT by default
	{
		switch (type)
		{
		case D3D12_COMMAND_LIST_TYPE_COPY:
			return m_copyCommandQueue;

		case D3D12_COMMAND_LIST_TYPE_COMPUTE:
			return m_computeCommandQueue;

		default:
			return m_directCommandQueue;
		}
	}

	// TODO this should be removed probably just misleading (we should get the command list from the command queue)
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> Mizu::Application::getCommandList(D3D12_COMMAND_LIST_TYPE type) // type = D3D12_COMMAND_LIST_TYPE_DIRECT by default
	{
		switch (type)
		{
		case D3D12_COMMAND_LIST_TYPE_COPY:
			return m_copyCommandQueue->getCommandList();

		case D3D12_COMMAND_LIST_TYPE_COMPUTE:
			return m_computeCommandQueue->getCommandList();

		default:
			return m_directCommandQueue->getCommandList();
		}
	}


	Microsoft::WRL::ComPtr<IDXGIAdapter4> Application::getAdapter()
	{
		ComPtr<IDXGIFactory4> factory;
		UINT createFactoryFlags = 0;
#if defined(_DEBUG)
		createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
		// we use create factory 2 function since there's no such one for 4
		ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&factory)));

		ComPtr<IDXGIAdapter1>adapter1;
		ComPtr<IDXGIAdapter4>adapter4;

		if (m_useWarp)
		{
			ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter1)));
			ThrowIfFailed(adapter1.As(&adapter4));
		}
		else
		{
			SIZE_T bestVideoMemory = 0;
			for (UINT i = 0; factory->EnumAdapters1(i, &adapter1) != DXGI_ERROR_NOT_FOUND; ++i)
			{
				DXGI_ADAPTER_DESC1 adapterDesc;
				adapter1->GetDesc1(&adapterDesc);

				if ((adapterDesc.Flags && DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
					SUCCEEDED(D3D12CreateDevice(adapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) &&
					adapterDesc.DedicatedVideoMemory > bestVideoMemory)
				{
					bestVideoMemory = adapterDesc.DedicatedVideoMemory;
					ThrowIfFailed(adapter1.As(&adapter4));
				}
			}
		}

		return adapter4;
	}


	ComPtr<ID3D12Device2> Application::createDevice()
	{
		ComPtr<ID3D12Device2> device2;
		ThrowIfFailed(D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device2)));

#if defined(_DEBUG)
		ComPtr<ID3D12InfoQueue> infoQueue;
		if (SUCCEEDED(device2.As(&infoQueue)))
		{
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

			// NOTE this can be changed these later about what to suppress out of the warnings and errors
			// in case skipping a category is needed : D3D12_MESSAGE_CATEGORY Categories[] can be used
			D3D12_MESSAGE_SEVERITY severities[] = {
				D3D12_MESSAGE_SEVERITY_INFO // suppress only the info messages
			};
			D3D12_MESSAGE_ID DenyIds[] = {
				D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
				D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
				D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
			};

			D3D12_INFO_QUEUE_FILTER filter = {};

			//filter.DenyList.NumCategories = _countof(Categories);
			//filter.DenyList.pCategoryList = Categories;
			filter.DenyList.NumSeverities = _countof(severities);
			filter.DenyList.pSeverityList = severities;
			filter.DenyList.NumIDs = _countof(DenyIds);
			filter.DenyList.pIDList = DenyIds;

			ThrowIfFailed(infoQueue->PushStorageFilter(&filter));
		}
#endif
		return device2;
	}


	pair<ComPtr<ID3D12DescriptorHeap>, UINT> Application::createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors)
	{
		ComPtr<ID3D12DescriptorHeap> descriporHeap;
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = numDescriptors;
		desc.Type = type;
		//will leave desc.Flags for now
		ThrowIfFailed(m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriporHeap)));

		return { descriporHeap, getDescriptorHandleIncrementSize(type) };
	}


	UINT Application::getDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type)
	{
		return m_device->GetDescriptorHandleIncrementSize(type);
	}

	void Application::flush()
	{
		m_directCommandQueue->flush();
		m_copyCommandQueue->flush();
		m_computeCommandQueue->flush();
	}

	void Mizu::Application::close()
	{
		m_directCommandQueue->closeHandle();
		m_copyCommandQueue->closeHandle();
		m_computeCommandQueue->closeHandle();

		ms_windowsNameMap.clear();
		ms_windowsHwndMap.clear();
	}

        void Application::removeWindow(HWND hWnd)
        {
                auto it = ms_windowsHwndMap.find(hWnd);
                if (it != ms_windowsHwndMap.end())
                {
                        auto pWindow = it->second;
                        ms_windowsNameMap.erase(pWindow->m_name);
                        ms_windowsHwndMap.erase(it);
                }
        }

	uint64_t Mizu::Application::getFrameNumber() // static
	{
		return ms_frameNumber;
	}
}