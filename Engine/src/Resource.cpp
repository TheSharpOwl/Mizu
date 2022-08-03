#include <DX12LibPCH.h>
#include "Mizu/Resource.hpp"


namespace Mizu
{
    Resource::Resource(const std::wstring& name) {}
    Resource::Resource(const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue, const std::wstring& name) {}
    Resource::Resource(Microsoft::WRL::ComPtr<ID3D12Resource> resource, const std::wstring& name) {}
    Resource::Resource(const Resource& copy) {}
    Resource::Resource(Resource&& copy) {}
    Resource& Resource::operator=(const Resource& other) { return *this; }
    Resource& Resource::operator=(Resource&& copy) { return *this; }

	Resource::~Resource(){}


    void setResource(Microsoft::WRL::ComPtr<ID3D12Resource> resource, const D3D12_CLEAR_VALUE* clearValue)
    {
	    
    }

    void setName(const std::wstring& name)
    {
	    
    }

    void resetResource()
    {
	    
    }
}