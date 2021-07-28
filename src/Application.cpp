#include "Application.hpp"
#include "CommandQueue.hpp"
#include "Window.hpp"

using namespace Mizu;
using namespace Microsoft::WRL;
using namespace std;

static Application* App = nullptr;


Application::Application(HINSTANCE hInst) :
	m_hInstance(hInst),
	m_IsTearingSupported(false)
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

	// get the adapter
	m_adapter = GetAdapter();
	// make a device
	m_device = CreateDevice();
	// create the command queue
	// TODO create the other 2 types of command queues here
	m_commandQueue = make_shared<CommandQueue>(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
	// create the window and pass the command queue to it OR BETTER TO MAKE THE WINDOW	GET THE COMMAND QUEUE FROM APPLICATION!
    m_window = make_shared<Window>(L"MizuWindowClass", L"Mizu Demo", hInst, Width, Height);
}
void Application::Create(HINSTANCE hInst) // static 
{
	if (!App)
	{
		App = new Application(hInst);
	}
}

void Application::Destroy() // static
{

}
Application& Application::Get() // static
{
	return *App;
}

bool Application::IsTearingSupported() const
{
	return m_IsTearingSupported;
}

ComPtr<ID3D12Device2> Application::GetDevice() const
{
	return m_device;
}

ComPtr<ID3D12CommandQueue> Application::GetCommandQueue() const
{
	return m_commandQueue->GetCommandQueue();
}


Microsoft::WRL::ComPtr<IDXGIAdapter4> Application::GetAdapter()
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


ComPtr<ID3D12Device2> Application::CreateDevice()
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

		// TODO might change these later about what to suppress out of the warnings and errors
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