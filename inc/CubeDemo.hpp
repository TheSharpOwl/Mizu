#pragma once
#include "DX12LibPCH.h"

namespace Mizu
{
	class CubeDemo
	{
	public:

		CubeDemo(int width, int height, bool vsync);

		bool create(HINSTANCE hInst);
		bool LoadContent();
	private:
		
		void UpdateBufferResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
			ID3D12Resource** ppDestinationRes,
			ID3D12Resource** ppIntermediateRes,
			size_t numElements,
			size_t elementSize,
			const void* bufferData,
			D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

		int m_width;
		int m_height;
		bool m_vsync;

		// vertex buffer for our cube
		Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer;
		D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

	};

}

