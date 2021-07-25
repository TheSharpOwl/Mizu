#pragma once

#include "DX12LibPCH.h"
#include <memory>

class CommandQueue;

namespace Mizu
{
	class Window;

	class Application
	{
	public:

		Application() = delete;

		Application(const Application& copy) = delete;
		Application& operator=(const Application& other) = delete;


		uint32_t Width = 1080;
		uint32_t Height = 720;

		static void Create(HINSTANCE hInst);

		static void Destroy();

		static Application& Get();

		bool IsTearingSupported() const;

		Microsoft::WRL::ComPtr<ID3D12Device2> GetDevice() const;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue() const;

		
	protected:

		Application(HINSTANCE hInst);

		HINSTANCE m_hInstance;

		Microsoft::WRL::ComPtr<IDXGIAdapter4> m_adapter;
		Microsoft::WRL::ComPtr<ID3D12Device2> m_device;

		// TODO add 3 command queues : direct/compute/copy
		std::shared_ptr<CommandQueue> m_commandQueue;

		std::shared_ptr<Window> m_window;

		bool m_IsTearingSupported;
	};
}