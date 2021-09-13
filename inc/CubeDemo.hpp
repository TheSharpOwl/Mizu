#pragma once
#include "DX12LibPCH.h"

namespace Mizu
{
	struct ReizeEventArgs;

	class CubeDemo
	{
	public:

		CubeDemo(int width, int height, bool vsync);

		bool create(HINSTANCE hInst);
		bool LoadContent();

	protected:

		void OnResize(ReizeEventArgs& e);

	private:
	
		void UpdateBufferResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
			ID3D12Resource** ppDestinationRes,
			ID3D12Resource** ppIntermediateRes,
			size_t numElements,
			size_t elementSize,
			const void* bufferData,
			D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

		void ResizeDepthBuffer(int width, int height);

		int m_width;
		int m_height;
		bool m_vsync;

		// vertex buffer for our cube
		Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer;
		D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
		\
		Microsoft::WRL::ComPtr<ID3D12Resource> m_depthBuffer;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_piplineState;

		bool m_contentLoaded = false;

		D3D12_VIEWPORT m_viewPort;

	};

}

