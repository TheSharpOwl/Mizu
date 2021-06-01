
#include <Windows.h>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include "Helpers.h"

// DirectX 12 specific headers.
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

// D3D12 extension library.
#include "d3dx12.h"

#include "wrl.h"

using namespace Microsoft::WRL;
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
	windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
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

ComPtr<ID3D12CommandQueue> CreateCommandQueue(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE listType)
{
	ComPtr<ID3D12CommandQueue> queue;

	D3D12_COMMAND_QUEUE_DESC des;
	des.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	des.NodeMask = 0;
	des.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	des.Type = listType;

	ThrowIfFailed(device->CreateCommandQueue(&des, IID_PPV_ARGS(&queue)));
	return queue;
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

	DXGI_SWAP_CHAIN_DESC1 scDesc;
	scDesc.Width = width;
	scDesc.Height = height;
	scDesc.BufferCount = numberOfBuffers;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.Stereo = FALSE;
	scDesc.SampleDesc = { 0,1 }; // this must be so in the flip discard model https://en.wikipedia.org/wiki/Bit_blit
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	scDesc.Scaling = DXGI_SCALING_STRETCH;
	scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.Flags = checkTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
	ThrowIfFailed(f4->CreateSwapChainForHwnd(commandQueue.Get(), hWnd, &scDesc, nullptr, nullptr, &swapChain1));

	// disable alt + enter to handle fullscreen manually
	ThrowIfFailed(f4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));

	ThrowIfFailed(swapChain1.As(&swapChain4));
	return swapChain4;
}

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
	EnableDebugLayer();

	const wchar_t* windowClassName = L"MizuWindowClass";
	RegisterWindowClass(hInstance, windowClassName);
	HWND hWnd = CreateWindow(windowClassName, L"Mizu", hInstance, 300, 300);

	// TODO check warp support before passing useWarp argument
	ComPtr<IDXGIAdapter4> adapter = GetAdapter(true);

	auto device = CreateDevice(adapter);

	::ShowWindow(hWnd, SW_SHOW);

	// to print for example
	while (1)
	{
		OutputDebugStringW(
			L"Hello\n"		);
	}
	return 0;
}	