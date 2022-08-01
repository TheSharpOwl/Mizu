#pragma once
#include "d3d12.h"
#include "wrl.h"

#include <string>

namespace Mizu
{
    class Resource
    {
        Resource(const std::wstring& name = L"");
        Resource(const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue = nullptr, const std::wstring& name = L"");
        Resource(Microsoft::WRL::ComPtr<ID3D12Resource> resource, const std::wstring& name = L"");
        Resource(const Resource& copy);
        Resource(Resource&& copy);
        Resource& operator=(const Resource& other);
        Resource& operator=(Resource&& copy);



        virtual ~Resource();


        bool isValid() const
        {
            return m_resource != nullptr;
        }

    private:

        Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;

    };
}
