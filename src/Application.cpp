#include "Application.hpp"
#include "CommandQueue.hpp"

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

	// make a device

	// create the command queue

	// create the window and pass the command queue to it OR BETTER TO MAKE THE WINDOW	GET THE COMMAND QUEUE FROM APPLICATION!
//	m_window = make_shared<Window>(L"MizuWindowClass", L"Mizu Demo", hInst, Width, Height, m_commandQueue);
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

Microsoft::WRL::ComPtr<ID3D12Device2> Application::GetDevice() const
{
	return m_device;
}
Microsoft::WRL::ComPtr<ID3D12CommandQueue> Application::GetCommandQueue() const
{
	return m_commandQueue->GetCommandQueue();
}
