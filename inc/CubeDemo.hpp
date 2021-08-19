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
			D3D12_RESOURCE_FLAGS flags);

		int m_width;
		int m_height;
		bool m_vsync;
	};

}

