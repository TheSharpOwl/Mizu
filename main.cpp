
#include <Windows.h>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <chrono>
#include <stdio.h>
#include <wchar.h>
#include "Helpers.h"

// DirectX 12 specific headers.
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

// D3D12 extension library.
#include "d3dx12.h"
#include "wrl.h"

#include "CommandQueue.hpp"

using namespace Microsoft::WRL;

template <typename T>
using cp = ComPtr<T>;


template <typename T>
using sp = std::shared_ptr<T>;

uint32_t g_Width = 1280, g_Height = 720;

HWND g_hWnd;
// to toggle fullscreen state
RECT g_WindowRect;

HANDLE g_FenceEvent;

const int g_NumFrames = 2;
UINT g_CurrentBackBufferIndex = 0;

sp<CommandQueue> g_CommandQueue;

cp<IDXGIAdapter4> g_Adapter;
cp<ID3D12Device2> g_Device;
cp<ID3D12Resource> g_BackBuffers[g_NumFrames];
cp<ID3D12CommandAllocator> g_CommandAllocators[g_NumFrames];
cp<ID3D12GraphicsCommandList2> g_CommandList;
cp<ID3D12DescriptorHeap> g_RTVDescriptorHeap;
cp<IDXGISwapChain4> g_SwapChain;
cp<ID3D12Fence> g_Fence;
uint64_t g_FrameFenceValues[g_NumFrames] = {};
uint64_t g_FenceValue = 0;

bool g_VSync; // TODO set to the suitable value or take it from the command line
bool g_TearingSupported; // this too xD TODO
bool g_IsFullscreen;
bool g_IsInitialized;
bool g_UseWarp = true;
UINT g_RTVDescriptorSize;

const wchar_t* windowClassName = L"MizuWindowClass";


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

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
	windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	windowClass.lpszMenuName = nullptr;
	windowClass.lpszClassName = windowClassName;
	windowClass.hIconSm = nullptr;

	static HRESULT hl = ::RegisterClassExW(&windowClass);
	assert(SUCCEEDED(hl));

}


HWND CreateWindow(const wchar_t* windowClassName, const wchar_t* windowTitle, HINSTANCE hInst, uint32_t windowWidth, uint32_t windowHeight)
{

	int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

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

void EnableDebugLayer()
{
#if defined(_DEBUG)
	ComPtr<ID3D12Debug> debugInterface;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
	debugInterface->EnableDebugLayer();
#endif
}


ComPtr<IDXGIAdapter4> GetAdapter(bool useWarp)
{
	ComPtr<IDXGIFactory4> dFactory;
	UINT createFactoryFlags = 0;
#if defined(_DEBUG)
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
	// we use create factory 2 function since there's no such one for 4
	ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dFactory)));

	ComPtr<IDXGIAdapter1>dAdapter1;
	ComPtr<IDXGIAdapter4>dAdapter4;

	if (useWarp)
	{
		ThrowIfFailed(dFactory->EnumWarpAdapter(IID_PPV_ARGS(&dAdapter1)));
		ThrowIfFailed(dAdapter1.As(&dAdapter4));
	}
	else
	{
		SIZE_T bestVideoMemory = 0;
		for (UINT i = 0; dFactory->EnumAdapters1(i, &dAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			DXGI_ADAPTER_DESC1 dAdapterDesc;
			dAdapter1->GetDesc1(&dAdapterDesc);

			if ((dAdapterDesc.Flags && DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
				SUCCEEDED(D3D12CreateDevice(dAdapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) &&
				dAdapterDesc.DedicatedVideoMemory > bestVideoMemory)
			{
				bestVideoMemory = dAdapterDesc.DedicatedVideoMemory;
				ThrowIfFailed(dAdapter1.As(&dAdapter4));
			}
		}
	}

	return dAdapter4;
}

ComPtr<ID3D12Device2> CreateDevice(ComPtr<IDXGIAdapter4> adapter4)
{
	ComPtr<ID3D12Device2> device2;
	ThrowIfFailed(D3D12CreateDevice(adapter4.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device2)));

#if defined(_DEBUG)
	ComPtr<ID3D12InfoQueue> infoQueue;
	if (SUCCEEDED(device2.As(&infoQueue)))
	{
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

		// TODO might change these later about what to supress out of the warnings and errors
		// in case skipping a categoty is needed : D3D12_MESSAGE_CATEGORY Categories[] can be used
		D3D12_MESSAGE_SEVERITY severities[] = {
			D3D12_MESSAGE_SEVERITY_INFO // supress only the info messages
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

bool checkTearingSupport()
{
	// using factory 1.4 then 1.5 to enable graphics debugging tool which are not supported (at least until the tutorial date so I will check it later) TODO
	BOOL allowTearing = FALSE;
	ComPtr<IDXGIFactory4> f4;
	if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&f4))))
	{
		ComPtr<IDXGIFactory5> f5;
		if (SUCCEEDED(f4.As(&f5)))
		{
			if (FAILED(f5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing))))
			{
				allowTearing = FALSE;
			}
		}
	}

	return allowTearing == TRUE;
}

ComPtr<IDXGISwapChain4> CreateSwapChain(HWND hWnd, ComPtr<ID3D12CommandQueue> commandQueue, uint32_t width, uint32_t height, uint32_t numberOfBuffers)
{
	ComPtr<IDXGIFactory4> f4;
	// well we have to create a swapchain1 and cast it to a swapchain4 because the CreateSwapChainForHwnd func takes swapchain1.......
	ComPtr<IDXGISwapChain1> swapChain1;
	ComPtr<IDXGISwapChain4> swapChain4;

	UINT createFactoryFlags = 0;
#if defined(_DEBUG)
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&f4)));
	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	scDesc.Width = width;
	scDesc.Height = height;
	scDesc.BufferCount = numberOfBuffers;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.Stereo = FALSE;
	scDesc.SampleDesc = {1, 0 }; // this must be so in the flip discard model https://en.wikipedia.org/wiki/Bit_blit
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	scDesc.Scaling = DXGI_SCALING_STRETCH;
	scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.Flags = checkTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	ThrowIfFailed(f4->CreateSwapChainForHwnd(commandQueue.Get(), hWnd, &scDesc, nullptr, nullptr, &swapChain1));

	// disable alt + enter to handle full screen manually
	ThrowIfFailed(f4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));

	ThrowIfFailed(swapChain1.As(&swapChain4));
	return swapChain4;
}

ComPtr<ID3D12DescriptorHeap>CreateDescriptorHeap(ComPtr<ID3D12Device2> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors)
{
	ComPtr<ID3D12DescriptorHeap> descriporHeap;
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = numDescriptors;
	desc.Type = type;
	//will leave desc.Flags for now
	ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriporHeap)));

	return descriporHeap;
}

// the heap here is rtv type (from name but lol)
void UpdateRenderTargetViews(cp<ID3D12Device2> device, cp<IDXGISwapChain4> swapChain, cp<ID3D12DescriptorHeap> descriptorHeap)
{
	auto rtvDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	// here we assume it's for cpu
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart());
	for (int i = 0; i < g_NumFrames; i++)
	{
		cp<ID3D12Resource> backBuffer;
		ThrowIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));
		device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);
		g_BackBuffers[i] = backBuffer;
		rtvHandle.Offset(rtvDescSize);
	}
}

void Update()
{

	static uint64_t frameCounter = 0;
	static double secondsPassed = 0;
	static std::chrono::high_resolution_clock clock;
	static auto t0 = clock.now();

	frameCounter++;
	auto t1 = clock.now();
	auto delta = t1 - t0;
	t0 = t1;

	secondsPassed += delta.count() * 1e-9;

	if (secondsPassed > 1.0)
	{
		wchar_t buffer[500];
		auto fps = frameCounter / secondsPassed;
		swprintf_s(buffer, 500, L"FPS: %f\n", fps);
		OutputDebugString(buffer);

		frameCounter = 0;
		secondsPassed = 0.0;
	}
}

void Render()
{
	auto& backBuffer = g_BackBuffers[g_CurrentBackBufferIndex];

	g_CommandList = g_CommandQueue->GetCommandList();
	// clearing the render target
	{

		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		g_CommandList->ResourceBarrier(1, &barrier);

		FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(g_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), g_CurrentBackBufferIndex, g_RTVDescriptorSize);
		g_CommandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
	}
	// presenting
	{
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		g_CommandList->ResourceBarrier(1, &barrier);
		
		g_FenceValue = g_FrameFenceValues[g_CurrentBackBufferIndex] = g_CommandQueue->ExecuteCommandList(g_CommandList);
		UINT syncInterval = g_VSync ? 1 : 0;
		UINT presentFlags = g_TearingSupported && !g_VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
		ThrowIfFailed(g_SwapChain->Present(syncInterval, presentFlags));

		

		g_CurrentBackBufferIndex = g_SwapChain->GetCurrentBackBufferIndex();
		g_CommandQueue->WaitForFenceValue(g_FrameFenceValues[g_CurrentBackBufferIndex]);
	}
	
}

void Resize(uint32_t width, uint32_t height)
{
	if (g_Width == width && g_Height == height)
		return;
	g_Width = std::max(width, 1U);
	g_Height = std::max(height, 1U);

	g_CommandQueue->Flush();

	// this is not necessary afaik TODO check that
	for (uint32_t i = 0; i < g_NumFrames; i++)
	{
		g_BackBuffers[i].Reset();
		g_FrameFenceValues[i] = g_FrameFenceValues[g_CurrentBackBufferIndex];
	}


	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	ThrowIfFailed(g_SwapChain->GetDesc(&swapChainDesc));
	ThrowIfFailed(g_SwapChain->ResizeBuffers(g_NumFrames, width, height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

	g_CurrentBackBufferIndex = g_SwapChain->GetCurrentBackBufferIndex();
	UpdateRenderTargetViews(g_Device, g_SwapChain, g_RTVDescriptorHeap);
}

void SetFullscreen(bool isFullscreen)
{
	if (isFullscreen == g_IsFullscreen)
		return;
	if (isFullscreen)// to full screen
	{
		// store the window dimensions
		::GetWindowRect(g_hWnd, &g_WindowRect);
		
		// set the window to a style such that it is border-less 
		UINT windowStyle = WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);

		::SetWindowLongW(g_hWnd, GWL_STYLE, windowStyle);

		// get the dimensions of the full screen state by getting the monitor dim
		HMONITOR hMonitor = ::MonitorFromWindow(g_hWnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFOEX monitorInfo = {};
		monitorInfo.cbSize = sizeof(MONITORINFOEX);
		::GetMonitorInfo(hMonitor, &monitorInfo);

		::SetWindowPos(g_hWnd, HWND_TOP, monitorInfo.rcMonitor.left,
			monitorInfo.rcMonitor.top,
			monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
			monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);


		::ShowWindow(g_hWnd, SW_MAXIMIZE);
	}
	else // to windowed mode
	{
		::SetWindowLongW(g_hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);

		::SetWindowPos(g_hWnd, HWND_NOTOPMOST,
			g_WindowRect.left,
			g_WindowRect.top,
			g_WindowRect.right - g_WindowRect.left,
			g_WindowRect.bottom - g_WindowRect.top,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);

		::ShowWindow(g_hWnd, SW_NORMAL);
	}

	g_IsFullscreen = isFullscreen;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (g_IsInitialized)
	{
		switch (message)
		{
		case WM_PAINT:
			Update();
			Render();
			break;

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			bool alt = (::GetAsyncKeyState(VK_MENU) & 0x8000) != 0;

			switch (wParam)
			{
			case 'V':
				g_VSync = !g_VSync;
				break;
			case VK_ESCAPE:
				::PostQuitMessage(0);
				break;
			case VK_RETURN:
				if (alt)
				{
			case VK_F11:
				SetFullscreen(!g_IsFullscreen);
				}
				break;
			}
		}
		break;
		// this case is for not getting a windows annoying sound
		case WM_SYSCHAR:
			break;

		case WM_SIZE:
		{
			RECT clientRect = {};
			::GetClientRect(g_hWnd, &clientRect);

			int width = clientRect.right - clientRect.left;
			int height = clientRect.bottom - clientRect.top;

			Resize(width, height);
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

	// Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
	// Using this awareness context allows the client area of the window 
	// to achieve 100% scaling while still allowing non-client window content to 
	// be rendered in a DPI sensitive fashion.
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	EnableDebugLayer();

	g_TearingSupported = checkTearingSupport();

	const wchar_t* windowClassName = L"MizuWindowClass";
	RegisterWindowClass(hInstance, windowClassName);
	HWND g_hWnd = CreateWindow(windowClassName, L"Mizu", hInstance, g_Width, g_Height);

	::GetWindowRect(g_hWnd, &g_WindowRect);

	// TODO check warp support before passing useWarp argument
	g_Adapter = GetAdapter(g_UseWarp);

	g_Device = CreateDevice(g_Adapter);
	g_CommandQueue = std::make_shared<CommandQueue>(g_Device, D3D12_COMMAND_LIST_TYPE_DIRECT);

	g_SwapChain = CreateSwapChain(g_hWnd, g_CommandQueue->GetCommandQueue(), g_Width, g_Height, g_NumFrames);


	g_CurrentBackBufferIndex = g_SwapChain->GetCurrentBackBufferIndex();

	g_RTVDescriptorHeap = CreateDescriptorHeap(g_Device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, g_NumFrames);
	g_RTVDescriptorSize = g_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	UpdateRenderTargetViews(g_Device, g_SwapChain, g_RTVDescriptorHeap);

	g_IsInitialized = true;

	::ShowWindow(g_hWnd, SW_SHOW);
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

	g_CommandQueue->Flush();

	::CloseHandle(g_FenceEvent);

	return 0;
}