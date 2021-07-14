#pragma once

#include <memory>
#include <string>

#include "d3dx12.h"
#include <dxgi1_6.h>
#include <wrl.h>


class Window;
class Game;
class CommandQueue;


class Application
{
public:

	
	static void Create(HINSTANCE hInst);

	static void Destroy();


	static Application& Get();


	bool IsTearingSupported() const;

	std::shared_ptr<Window> CreateRenderWindow(const std::wstring& windowName, int width, int height, bool vsync = true);


	void DestroyWindow(const std::wstring& windowName);

	void DestroyWindow(std::shared_ptr<Window> window_ptr);

	std::shared_ptr<Window> getWindow(const std::wstring& windowName);

	int Run(std::shared_ptr<Game> gamePtr);

	void Quit(int exitCode = 0);


	Microsoft::WRL::ComPtr<ID3D12Device2> GetDevice() const;


	Microsoft::WRL::ComPtr<CommandQueue> GetCommandQueue(D3D12_COMMAND_LIST_TYPE = D3D12_COMMAND_LIST_TYPE_DIRECT) const;

	void Flush();


	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type);

	UINT GetDescriptorHandleIncreamentSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const;


protected:

	Application(HINSTANCE hInst);

	virtual ~Application();

	Microsoft::WRL::ComPtr<IDXGIAdapter4> GetAdapter(bool useWarp);
	Microsoft::WRL::ComPtr<ID3D12Device2> CreateDevice(Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter);
	bool CheckTearingSupport();

private:

	Application(const Application& copy) = delete;
	Application& operator=(const Application& other) = delete;

	HINSTANCE m_hInstance;

	Microsoft::WRL::ComPtr<IDXGIAdapter4> m_Adapter;
	Microsoft::WRL::ComPtr<ID3D12Device2> m_Device;

	std::shared_ptr<CommandQueue> m_DirectCommandQueue;
	std::shared_ptr<CommandQueue> m_ComputeCommandQueue;
	std::shared_ptr<CommandQueue> m_CopyCommandQueue;

	bool m_TearingSupported;
};

