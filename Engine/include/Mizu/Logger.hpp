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

	// TODO this method does not work for my hardware but left here for learning (I think it can be fixed or it will be removed later)
	inline void monitors(Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter1)
	{
		Microsoft::WRL::ComPtr<IDXGIOutput> dxOutput;
		if (FAILED(adapter1->EnumOutputs(0, &dxOutput))) { return; }
		UINT modesCount = 0;
		DXGI_MODE_DESC* displayModes = NULL;
		DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT;

		// get the number of modes by passing the last parameter as null
		ThrowIfFailed(dxOutput->GetDisplayModeList(format, 0, &modesCount, nullptr));

		displayModes = new DXGI_MODE_DESC[modesCount];
		dxOutput->GetDisplayModeList(format, 0, &modesCount, displayModes);

		// get the adapter name for the output
		DXGI_ADAPTER_DESC desc;
		adapter1->GetDesc(&desc);

		std::wstring textLine = L"Adapter " + std::wstring(desc.Description) + L"monitors connected:\n";

		for(UINT i = 0 ; i < modesCount; i++)
		{
			textLine = L"Monitor " + std::to_wstring(i);
			textLine += L" Width " + std::to_wstring(displayModes[i].Width);
			textLine += L" Height " + std::to_wstring(displayModes[i].Height);
			textLine += L" " + std::to_wstring(displayModes[i].RefreshRate.Numerator);
			textLine += L"/" + std::to_wstring(displayModes[i].RefreshRate.Denominator) + L" Hz\n";

			OutputDebugStringW(textLine.c_str());
		}

		free(displayModes);
	}
}
