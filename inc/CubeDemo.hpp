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
		
		int m_width;
		int m_height;
		bool m_vsync;
	};

}

