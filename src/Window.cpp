#include "Window.hpp"

using namespace Mizu;
using namespace Microsoft::WRL;
using namespace std;

Window::Window(const wchar_t* windowClassName, const wchar_t* windowTitle, HINSTANCE hInst, uint32_t windowWidth, uint32_t windowHeight, shared_ptr<CommandQueue> commandQueue) :
	screenWidth(::GetSystemMetrics(SM_CXSCREEN)), screenHeight(::GetSystemMetrics(SM_CYSCREEN))
{
	RegisterWindowClass(hInst, windowClassName);

	windowRect = { 0, 0, static_cast<LONG>(windowWidth), static_cast<LONG>(windowHeight) };
	::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	this->windowWidth = windowRect.right - windowRect.left;
	this->windowHeight = windowRect.bottom - windowRect.top;

	// To put the window in the center
	int windowX = std::max<int>(0, (screenWidth - windowWidth) / 2);
	int windowY = std::max<int>(0, (screenHeight - windowHeight) / 2);

	hWnd = ::CreateWindowExW(NULL, windowClassName, windowTitle, WS_OVERLAPPEDWINDOW, windowX, windowY, windowWidth, windowHeight, NULL, NULL, hInst, nullptr);

	/*DWORD d = GetLastError();*/ //left in case of debugging an error

	assert(hWnd && "Failed To create a window");

	m_swapChain = CreateSwapChain(commandQueue->GetCommandQueue());
}

void Window::RegisterWindowClass(HINSTANCE hInstance, const wchar_t* windowClassName)
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


bool Window::checkTearingSupport()
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

ComPtr<IDXGISwapChain4> Window::CreateSwapChain(Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue)
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
	scDesc.Width = windowWidth;
	scDesc.Height = windowHeight;
	scDesc.BufferCount = numberOfBuffers;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.Stereo = FALSE;
	scDesc.SampleDesc = { 1, 0 }; // this must be so in the flip discard model https://en.wikipedia.org/wiki/Bit_blit
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