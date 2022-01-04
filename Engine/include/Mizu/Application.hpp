#pragma once

#include "DX12LibPCH.h"
#include <memory>
#include <utility>

#include <unordered_map>

class CommandQueue;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

namespace Mizu
{
	class Window;
	class Game;

	class Application
	{
	public:
		
		static const wchar_t* windowClassName;

		Application() = delete;

		Application(const Application& copy) = delete;
		Application& operator=(const Application& other) = delete;

		static void Create(HINSTANCE hInst);

		std::shared_ptr<Window> createRenderWindow(const std::wstring& appName, int width, int height);

		int Run(std::shared_ptr<Game> game);

		static void Destroy();

		void DestroyWindow(const std::wstring& name);

		static Application& Get();

		bool IsTearingSupported() const;

		void Flush();
		
		void Close();

		Microsoft::WRL::ComPtr<ID3D12Device2> GetDevice() const;
		std::shared_ptr<CommandQueue> GetCommandQueue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT) const;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> GetCommandList(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);

		// return the created descriptor heap with its size
		std::pair<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>, UINT> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors);

		UINT GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type);

		auto getWindow(const std::wstring name) -> std::shared_ptr<Window> { auto it = m_windowsNameMap.find(name);  return (it != m_windowsNameMap.end() ? it->second : nullptr); }
		auto getWindow(HWND hWnd) -> std::shared_ptr<Window> { auto it = m_windowsHwndMap.find(hWnd);  return (it != m_windowsHwndMap.end() ? it->second : nullptr); }

	protected:

		Application(HINSTANCE hInst);

		Microsoft::WRL::ComPtr<IDXGIAdapter4> GetAdapter();


		Microsoft::WRL::ComPtr<ID3D12Device2> CreateDevice();

		HINSTANCE m_hInstance;

		Microsoft::WRL::ComPtr<IDXGIAdapter4> m_adapter;
		Microsoft::WRL::ComPtr<ID3D12Device2> m_device;

		std::shared_ptr<CommandQueue> m_directCommandQueue;
		std::shared_ptr<CommandQueue> m_copyCommandQueue;
		std::shared_ptr<CommandQueue> m_computeCommandQueue;

		std::unordered_map<std::wstring, std::shared_ptr<Window>> m_windowsNameMap;
		std::unordered_map<HWND, std::shared_ptr<Window>> m_windowsHwndMap;

		bool m_IsTearingSupported;

		const bool m_useWarp = false;

		friend LRESULT CALLBACK::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	};
}