#pragma once

#include <dxgi.h>
#include <dxgi1_2.h>
#include <string>
#include <wrl/client.h>
#include "Helpers.h"

namespace Mizu::Logger
{
	inline void adapters()
	{
		using namespace Microsoft::WRL;
		ComPtr<IDXGIFactory2> factory;
		ComPtr<IDXGIAdapter1> adapter;

		UINT createFactoryFlags = 0;
#if defined(_DEBUG)
		createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

		ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&factory)));
		for(UINT i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			DXGI_ADAPTER_DESC desc;
			adapter->GetDesc(&desc);

			std::wstring textLine = L"Adapter " + std::to_wstring(i) + L" " + desc.Description + L"\n";

			OutputDebugStringW(textLine.c_str());
		}
	}
}
