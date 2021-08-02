#pragma once

#include "DX12LibPCH.h"
#include <memory>
#include <utility>

class CommandQueue;

namespace Mizu
{

	class Window;
	class Application
	{
	public:
		
		static inline const wchar_t* windowClassName = L"MizuWindowClass";

		Application() = delete;

		Application(const Application& copy) = delete;
		Application& operator=(const Application& other) = delete;


		uint32_t Width = 1080;
		uint32_t Height = 720;

		static void Create(HINSTANCE hInst);

		static void Destroy();

		static Application& Get();

		bool IsTearingSupported() const;

		void Flush();
		
		void Close();

		Microsoft::WRL::ComPtr<ID3D12Device2> GetDevice() const;
		std::shared_ptr<CommandQueue> GetCommandQueue() const;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> GetCommandList();

		// return the created descriptor heap with its size
		std::pair<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>, UINT> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors);

		UINT GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type);

	protected:

		Application(HINSTANCE hInst);

		Microsoft::WRL::ComPtr<IDXGIAdapter4> GetAdapter();


		Microsoft::WRL::ComPtr<ID3D12Device2> CreateDevice();

		HINSTANCE m_hInstance;

		Microsoft::WRL::ComPtr<IDXGIAdapter4> m_adapter;
		Microsoft::WRL::ComPtr<ID3D12Device2> m_device;

		// TODO add 3 command queues : direct/compute/copy
		std::shared_ptr<CommandQueue> m_commandQueue;

		std::shared_ptr<Window> m_window;

		bool m_IsTearingSupported;

		const bool m_useWarp = false;
	};
}