#include "Window.hpp"
//#include "DX12lib."
#include "Application.hpp"
#include <map>

template<typename T>
using sp = shared_ptr<T>;

template<typename T>
using cp = Microsoft::WRL::ComPtr<T>;

static std::map<sp<Window>, HWND> g_windowHWndMap;
static std::map<std::wstring, sp<Window>> g_NameWindowMap;



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
	wndClass.lpszClassName = L"Mizu Demo";
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
		assert()
	}
}

Application& Application::Get()
{
	// TODO: insert return statement here
}

bool Application::IsTearingSupported() const
{

}

std::shared_ptr<Window> Application::CreateRenderWindow(const std::wstring& windowName, int width, int height, bool vsync /*= true*/)
{

}

void Application::DestroyWindow(const std::wstring& windowName)
{

}

void Application::DestroyWindow(std::shared_ptr<Window> window_ptr)
{

}

std::shared_ptr<Window> Application::getWindow(const std::wstring& windowName)
{

}

int Application::Run(std::shared_ptr<Game> gamePtr)
{
	return 0;
}




void Application::Quit(int exitCode = 0) {}


Microsoft::WRL::ComPtr<ID3D12Device2> Application::GetDevice() const {}


Microsoft::WRL::ComPtr<CommandQueue> Application::GetCommandQueue(D3D12_COMMAND_LIST_TYPE = D3D12_COMMAND_LIST_TYPE_DIRECT) const {}

void Application::Flush() {}


Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> Application::CreateDescriptorHeap(UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type) {}

UINT Application::GetDescriptorHandleIncreamentSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const {}


Microsoft::WRL::ComPtr<IDXGIAdapter4> Application::GetAdapter(bool useWarp) {}
Microsoft::WRL::ComPtr<ID3D12Device2> Application::CreateDevice(Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter) {}
bool Application::CheckTearingSupport() {}