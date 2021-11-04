#include "..\inc\Window.hpp"
#include "Window.hpp"
#include "Application.hpp"
#include "CommandQueue.hpp"
#include "Game.hpp"
#include <algorithm>

using namespace Microsoft::WRL;
using namespace std;
using namespace Mizu;

Window::Window(const wchar_t* windowClassName, const wchar_t* windowTitle, HINSTANCE hInst, uint32_t windowWidth, uint32_t windowHeight) :
	m_screenWidth(::GetSystemMetrics(SM_CXSCREEN)), m_screenHeight(::GetSystemMetrics(SM_CYSCREEN))
{
	// window class is already registered in app constructor

	m_windowRect = { 0, 0, static_cast<LONG>(windowWidth), static_cast<LONG>(windowHeight) };
	::AdjustWindowRect(&m_windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	m_windowWidth = m_windowRect.right - m_windowRect.left;
	m_windowHeight = m_windowRect.bottom - m_windowRect.top;

	// To put the window in the center
	int windowX = std::max<int>(0, (m_screenWidth - windowWidth) / 2);
	int windowY = std::max<int>(0, (m_screenHeight - windowHeight) / 2);

	m_hWnd = ::CreateWindowExW(NULL, windowClassName, windowTitle, WS_OVERLAPPEDWINDOW, windowX, windowY, windowWidth, windowHeight, NULL, NULL, hInst, nullptr);

	/*DWORD d = GetLastError();*/ //left in case of debugging an error

	m_name = windowTitle;

	assert(m_hWnd && "Failed To create a window");

	Application& app = Application::Get();

	m_swapChain = CreateSwapChain(app.GetCommandQueue());

	m_currentBackBufferIndex = GetCurrentBackBufferIndex();

	auto [descriptorHeap, descriptorSize] = Application::Get().CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, numberOfBuffers);
	m_RTVDescriptorHeap = descriptorHeap;
	m_RTVDescriptorSize = descriptorSize;

	UpdateRenderTargetViews();

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


bool Window::CheckTearingSupport()
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

UINT Window::GetCurrentBackBufferIndex()
{
	return m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
}

UINT Window::Present(const UINT syncInterval, const UINT presentFlags)
{
	ThrowIfFailed(m_swapChain->Present(syncInterval, presentFlags));
	return m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void Window::ShowWindow()
{
	::ShowWindow(m_hWnd, SW_SHOW);
}

D3D12_CPU_DESCRIPTOR_HANDLE Mizu::Window::GetRTV() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_currentBackBufferIndex, m_RTVDescriptorSize);
}

Microsoft::WRL::ComPtr<ID3D12Resource> Mizu::Window::getCurrentBackBuffer()
{
	return m_backBuffers[m_currentBackBufferIndex];
}

ComPtr<IDXGISwapChain4> Window::CreateSwapChain(shared_ptr<CommandQueue> command_queue_sptr)
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
	scDesc.Width = m_windowWidth;
	scDesc.Height = m_windowHeight;
	scDesc.BufferCount = numberOfBuffers;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.Stereo = FALSE;
	scDesc.SampleDesc = { 1, 0 }; // this must be so in the flip discard model https://en.wikipedia.org/wiki/Bit_blit
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	scDesc.Scaling = DXGI_SCALING_STRETCH;
	scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.Flags = CheckTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	// get the COM ptr
	auto commandQueue = command_queue_sptr->GetCommandQueue();

	ThrowIfFailed(f4->CreateSwapChainForHwnd(commandQueue.Get(), m_hWnd, &scDesc, nullptr, nullptr, &swapChain1));

	// disable alt + enter to handle full screen manually
	ThrowIfFailed(f4->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER));

	ThrowIfFailed(swapChain1.As(&swapChain4));
	return swapChain4;
}

void Window::UpdateRenderTargetViews()
{
	auto device = Application::Get().GetDevice();
	auto rtvDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	// here we assume it's for CPU
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	for (int i = 0; i < numberOfBuffers; i++)
	{
		ComPtr<ID3D12Resource> backBuffer;
		ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));
		device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);
		m_backBuffers[i] = backBuffer;
		rtvHandle.Offset(rtvDescSize);
	}
}


void Window::Resize(uint32_t newWidth, uint32_t newHeight)
{
	if (m_windowWidth == newWidth && m_windowHeight == newHeight)
		return;

	m_windowWidth = max(newWidth, 1U);
	m_windowHeight = max(newHeight, 1U);

	Application::Get().Flush();

	// this is not necessary afaik TODO check that
	for (uint32_t i = 0; i < numberOfBuffers; i++)
	{
		m_backBuffers[i].Reset();
		m_frameFenceValues[i] = m_frameFenceValues[m_currentBackBufferIndex];
	}


	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	ThrowIfFailed(m_swapChain->GetDesc(&swapChainDesc));
	ThrowIfFailed(m_swapChain->ResizeBuffers(numberOfBuffers, m_windowWidth, m_windowHeight, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

	m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
	UpdateRenderTargetViews();
}

void Window::Update()
{

	static uint64_t frameCounter = 0;
	static double secondsPassed = 0;
	static std::chrono::high_resolution_clock clock;
	static auto t0 = clock.now();
	static double totalPassed = 0;

	if (auto game = m_game.lock())
	{
		// TODO next time:
		// 
		// 1. fix and pass correct args and fix total time (done)
		// 2. fix position of the cube to be in the middle (done)
		// 3. fix rendering args
		// 4. fix fps (done)
		// 5. fix the destructors (done)
		// 6. add unordered_map for windows in application (done)
		// 7. fix exception when making window bigger for mat multiplication

		UpdateEventArgs updateEventArgs(secondsPassed, totalPassed + secondsPassed);
		game->OnUpdate(updateEventArgs);
	}

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
		totalPassed += secondsPassed;
		frameCounter = 0;
		secondsPassed = 0.0;
	}


}


void Window::Render()
{
	if (auto game = m_game.lock())
	{
		// TODO fix and pass correct args
		RenderEventArgs r(0.f, 0.f);
		game->OnRender(r);
	}
}

void Mizu::Window::SetGamePtr(std::shared_ptr<Game> game)
{
	m_game = game;
}


