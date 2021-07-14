#include <DX12lib>
#include "Application.hpp"
#include <Window.h>


template<typename T>
using sp = shared_ptr<T>;

template<typename T>
using cp = Microsoft::WRL::ComPtr<T>;

sp<Window,HWND> windowHWndMap  


// TODO make it a smart pointer
static Application* AppSingelton = nullptr;


static 

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


Application::Application(HINSTANCE hInst) {
}

Application::~Application() {}

Microsoft::WRL::ComPtr<IDXGIAdapter4> Application::GetAdapter(bool useWarp) {}
Microsoft::WRL::ComPtr<ID3D12Device2> Application::CreateDevice(Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter) {}
bool Application::CheckTearingSupport() {}