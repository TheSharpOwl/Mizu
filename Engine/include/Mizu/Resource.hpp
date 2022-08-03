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

        Microsoft::WRL::ComPtr<ID3D12Resource> getResource() const
        {
            return m_resource;
        }

        D3D12_RESOURCE_DESC getResourceDesc() const
        {
            D3D12_RESOURCE_DESC result = {};
            if(m_resource)
            {
                result = m_resource->GetDesc();
            }

            return result;
        }

        virtual void setResource(Microsoft::WRL::ComPtr<ID3D12Resource> resource, const D3D12_CLEAR_VALUE* clearValue = nullptr);

        virtual D3D12_CPU_DESCRIPTOR_HANDLE getShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr) const = 0;

        virtual D3D12_CPU_DESCRIPTOR_HANDLE getUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc = nullptr) const = 0;

        void setName(const std::wstring& name);

        virtual void resetResource();

    private:

        Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
        std::unique_ptr<D3D12_CLEAR_VALUE> m_clearValue;
        std::wstring m_resourceName;
    };
}
