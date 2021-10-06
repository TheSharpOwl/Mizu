#pragma once
#include "DX12LibPCH.h"
#include "Game.hpp"
#include "Window.hpp"


namespace Mizu
{
	struct ReizeEventArgs;
	struct UpdateEventArgs;
	struct RenderEventArgs;

	class CubeDemo : public Game
	{
		template<typename T>
		using cp = Microsoft::WRL::ComPtr<T>;

	public:

		CubeDemo(int width, int height, bool vsync);

		bool LoadContent();

		void UnloadContent() override;

	protected:

		void OnResize(ReizeEventArgs& e) override;

		void OnUpdate(UpdateEventArgs& e) override;

		void OnRender(RenderEventArgs& e) override;

	private:
	
		void UpdateBufferResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
			ID3D12Resource** ppDestinationRes,
			ID3D12Resource** ppIntermediateRes,
			size_t numElements,
			size_t elementSize,
			const void* bufferData,
			D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

		void ResizeDepthBuffer(int width, int height);

		void TransitionResource(cp<ID3D12GraphicsCommandList2> commandList, cp<ID3D12Resource> resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);

		void ClearRTV(cp<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor);

		void ClearDepth(cp<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth = 1.0f);

		// vertex buffer for our cube
		Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer;
		D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_depthBuffer;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_piplineState;

		D3D12_VIEWPORT m_viewport;

		float m_fov;

		bool m_contentLoaded = false;

		DirectX::XMMATRIX m_modelMatrix;
		DirectX::XMMATRIX m_viewMatrix;
		DirectX::XMMATRIX m_projectionMatrix;

		D3D12_RECT m_scissorRect;

		int64_t m_fenceValues[Window::numberOfBuffers] = {};
	};

}

