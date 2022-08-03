#pragma once
#include "d3d12.h"
#include "wrl.h"

#include <string>

namespace Mizu
{
	/**
	 * \brief wrapper for direct3d 12 resource and provides a base class for other types of resources such as textures and buffers
	 */
	class Resource
    {
        Resource(std::wstring name = L"");

        Resource(const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue = nullptr, const std::wstring& name = L"");

        Resource(Microsoft::WRL::ComPtr<ID3D12Resource> resource, const std::wstring& name = L"");


        Resource(const Resource& copy);
        Resource(Resource&& copy);


        Resource& operator=(const Resource& other);
        Resource& operator=(Resource&& other);


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

        // should only be called by the command list and the name will stay the same unless setName(std::wstring) is called
        virtual void setResource(Microsoft::WRL::ComPtr<ID3D12Resource> resource, const D3D12_CLEAR_VALUE* clearValue = nullptr);

        /**
         * \brief returns the SRV for a resource
         * \param srvDesc description of the SRV to return. (default is nullptr which returns the default SRV for the resource (the SRV that is created with no description provided)
         * \return 
         */
        virtual D3D12_CPU_DESCRIPTOR_HANDLE getShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr) const = 0;

        /**
         * \brief get UAV for a (sub)resource
         * \param uavDesc  description of the UAV to return
         * \return 
         */
        virtual D3D12_CPU_DESCRIPTOR_HANDLE getUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc = nullptr) const = 0;

        void setName(const std::wstring& name);


        // release the underlying resource which is useful for swap chain resizing
        virtual void resetResource();

    private:

        Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
        std::unique_ptr<D3D12_CLEAR_VALUE> m_clearValue;
        std::wstring m_resourceName;
    };
}
