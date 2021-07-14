#include "Window.hpp"
#include "Application.hpp"

template<typename T>
using cp = Microsoft::WRL::ComPtr<T>;

HWND Window::GetWindowHandle() const
{
    return m_hWnd;
}

void Window::Destroy()
{
}

const std::wstring& Window::GetWindowName() const
{
    return m_WindowName;
}

bool Window::IsVSync() const
{
    return false;
}

void Window::SetVSync(bool vSync)
{
    m_VSync = vSync;
}

bool Window::IsFullScreen() const
{
    return m_FullScreen;
}

void Window::SetFullScreen(bool fullscreen)
{
    // TODO requires extra steps (it's already defined in main just copy paste is needed)
}

void Window::ToggleFullScreen()
{

}

void Window::Show()
{
}

void Window::Hide()
{
}

UINT Window::GetCurrentBackBufferIndex() const
{
    return m_CurrentBackBufferIndex;
}

UINT Window::Present()
{
	UINT syncInterval = m_VSync ? 1 : 0;
	UINT presentFlags = m_TearingSupported && !g_VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
	ThrowIfFailed(m_SwapChain->Present(syncInterval, presentFlags));

    m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

    return m_CurrentBackBufferIndex;
}

D3D12_CPU_DESCRIPTOR_HANDLE Window::GetCurrentTargetRenderView() const
{
	return D3D12_CPU_DESCRIPTOR_HANDLE(m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_CurrentBackBufferIndex, m_RTVDescriptorSize);
}

Microsoft::WRL::ComPtr<ID3D12Resource> Window::GetCurrentBackBuffer() const
{
    return Microsoft::WRL::ComPtr<ID3D12Resource>();
}

Microsoft::WRL::ComPtr<IDXGISwapChain4> Window::CreateSwapChain()
{

    cp<IDXGIFactory4> f4;
    cp<IDXGISwapChain4> swapChain4;
    cp<IDXGISwapChain2> swapChain2;

    UINT factoryFlags = 0;

#if defined(DEBUG)
    factoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    ThrowIfFailed(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&f4)));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = m_ClientWidth;
    swapChainDesc.Height = m_ClientHeight;
    swapChainDesc.BufferCount = BufferCount;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc = {1, 0};
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Flags = m_TearingSupported ? DXGI_PRESENT_ALLOW_TEARING : 0;

    // TODO add the command queue getting from the app class


    return Microsoft::WRL::ComPtr<IDXGISwapChain4>();
}



Window::Window(HWND hWnd, const std::wstring& windowName, int width, int height, bool vsync) :
        m_hWnd(hWnd)
		, m_WindowName(windowName)
		, m_ClientWidth(clientWidth)
		, m_ClientHeight(clientHeight)
		, m_VSync(vSync)
		, m_Fullscreen(false)
		, m_FrameCounter(0)
{
    
}

Window::~Window()
{
}

void Window::OnUpdate(UpdateEventArgs& e){}
void Window::OnRender(RenderEventArgs& e){}
void Window::OnKeyPressed(KeyEventArgs& e){}
void Window::OnKeyReleased(KeyEventArgs& e){}
void Window::OnMouseMoved(MouseMotionEventArgs& e){}
void Window::OnMouseButtonPressed(MouseButtonEventArgs& e){}
void Window::OnMouseButtonReleased(MouseButtonEventArgs& e){}
void Window::OnMouseWheel(MouseWheelEventArgs& e){}
void Window::OnResize(ResizeEventArgs& e){}

Microsoft::WRL::ComPtr<IDXGISwapChain4> CreateSwapChain(){}

void UpdateRenderTargetViews();