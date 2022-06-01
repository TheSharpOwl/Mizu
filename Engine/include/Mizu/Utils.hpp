#pragma once
#include<string>
#include <atlstr.h>
#include <cassert>

namespace Mizu
{
    static std::wstring resources_dir = L"";

    void ParseCommandLineArguments()
    {
        int argc;
        wchar_t** argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);

        for (size_t i = 0; i < argc; ++i)
        {
            if (::wcscmp(argv[i], L"--res_dir") == 0)
            {
                resources_dir = argv[++i];
                OutputDebugString(L"Hey now you are an all star!\n");
                OutputDebugString(resources_dir.c_str());
            }
        }

        // Free memory allocated by CommandLineToArgvW
        ::LocalFree(argv);
    }

    std::wstring to_wstring(LPCSTR s)
    {
        std::string temp(s);
        std::wstring ans(temp.begin(), temp.end());
        return ans;
    }

	/// <summary>
	/// Compiles a shader to a shaderBolb comptr (out parameter)
	/// </summary>
	/// <param name="filename">IN name of the shader file to compile</param>
	/// <param name="entryPoint">IN name of the entry point function</param>
	/// <param name="targetType">IN type of the shader as in https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/specifying-compiler-targets </param>
	/// <param name="compileFlags">IN as in https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/d3dcompile-constants </param>
	/// <param name="shaderBlob">OUT a pointer to a pointer (or to the ComPtr) which will point to the shaderBlob object as a result</param>

   

	void CompileShader(LPCWSTR filename, LPCSTR entryPoint, LPCSTR targetType, UINT compileFlags, ID3DBlob** shaderBlob)
	{
        ParseCommandLineArguments(); // to get the resources directory

        ID3DBlob* errorMessages;
        auto temp = std::wstring(filename);
        LPCWSTR fullFileDir = temp.c_str();
        std::string temp_debug = std::string(temp.begin(), temp.end());
        HRESULT hr = D3DCompileFromFile(temp.c_str(), nullptr, nullptr, entryPoint, targetType, compileFlags, 0, shaderBlob, &errorMessages);

        if (FAILED(hr))
        {
            if (errorMessages)
            {
                wchar_t message[1024] = { 0 };
                char* blobdata = reinterpret_cast<char*>(errorMessages->GetBufferPointer());

                MultiByteToWideChar(CP_ACP, 0, blobdata, static_cast<int>(errorMessages->GetBufferSize()), message, 1024);
                std::wstring fullMessage = L"Error compiling shader type";
                fullMessage += to_wstring(targetType);
                fullMessage += L" ";
                fullMessage += filename;
                fullMessage += message;

                // Pop up a message box allowing user to retry compilation
                int retVal = MessageBoxW(nullptr, fullMessage.c_str(), L"Shader Compilation Error", MB_RETRYCANCEL);
                if (retVal != IDRETRY)
                {
                    std::string str(fullMessage.begin(), fullMessage.end());
                    throw std::runtime_error(str);
                }
            }
        }
	}
    /// <summary>
    /// Maps t from range [x,y] to [-1,1] (screen coordinates
    /// </summary>
    /// <param name="t"></param>
    /// <param name="x"></param>
    /// <param name="y"></param>
    /// <returns></returns>
    float mapToScreen(float t, float x, float y)
    {
        assert((t >= x && t <= y) && "t should be in range [x,y]");
        return (2.f * t) / (y - x) - 1;
    }

    template<typename T>
    Microsoft::WRL::ComPtr<ID3D12Resource> createDefaultBuffer(ID3D12Device* device, const std::vector<T>& data, D3D12_RESOURCE_STATES finalState, std::wstring name = L"")
    {
        UINT elementSize = sizeof(T);
        UINT bufferSize = elementSize * data.size();

        D3D12_HEAP_PROPERTIES heapProps;
        ZeroMemory(&heapProps, sizeof(heapProps));
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN; // if cpu can access the heap or not (read and/or write or none)
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN; // this affects the bandwidth of the memory (faster for cpu, slower for gpu or vice versa... check the reference page)
        heapProps.CreationNodeMask = 1; // where it should be created (in case of multi-adapter)
        heapProps.VisibleNodeMask = 1;// same but for visiblity not creation

        D3D12_RESOURCE_DESC resDesc;
        ZeroMemory(&resDesc, sizeof(resDesc));
        resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resDesc.Alignment = 0; // will use the default 64 kb
        resDesc.Width = bufferSize;
        resDesc.Height = 1;
        resDesc.DepthOrArraySize = 1;
        resDesc.MipLevels = 1;
        resDesc.Format = DXGI_FORMAT_UNKNOWN;
        resDesc.SampleDesc.Count = 1;
        resDesc.SampleDesc.Quality = 0;
        resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        // this heap stores the data inside the gpu
        Microsoft::WRL::ComPtr<ID3D12Resource> defaultBuffer;
        ThrowIfFailed(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&defaultBuffer)));

        defaultBuffer->SetName(name.c_str());

        heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

        // this heap will upload the data to the default buffer (because cpu cannot access the default heap)
        Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;
        ThrowIfFailed(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer)));


        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
        ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));



        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
        ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(),nullptr, IID_PPV_ARGS(&commandList)));

        D3D12_COMMAND_QUEUE_DESC queueDesc;
        ZeroMemory(&queueDesc, sizeof(queueDesc));
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.NodeMask = 0;


        Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
        ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));

        void* pData;
        ThrowIfFailed(uploadBuffer->Map(0, NULL, &pData));

        memcpy(pData, data.data(), bufferSize);
        uploadBuffer->Unmap(0, NULL);

        commandList->CopyBufferRegion(defaultBuffer.Get(), 0, uploadBuffer.Get(), 0, bufferSize);


        D3D12_RESOURCE_BARRIER barrierDesc;
        ZeroMemory(&barrierDesc, sizeof(barrierDesc));
        barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrierDesc.Transition.pResource = defaultBuffer.Get();
        barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrierDesc.Transition.StateAfter = finalState;
        barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        commandList->ResourceBarrier(1, &barrierDesc);

        commandList->Close();
        std::vector<ID3D12CommandList*> ppCommandLists{ commandList.Get() };
        commandQueue->ExecuteCommandLists(static_cast<UINT>(ppCommandLists.size()), ppCommandLists.data());

        UINT64 initialValue{ 0 };
        Microsoft::WRL::ComPtr<ID3D12Fence> fence;
        if (FAILED(device->CreateFence(initialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.ReleaseAndGetAddressOf()))))
        {
            throw(std::runtime_error{ "Error creating a fence." });
        }

        HANDLE fenceEventHandle{ CreateEvent(nullptr, FALSE, FALSE, nullptr) };
        if (fenceEventHandle == NULL)
        {
            throw(std::runtime_error{ "Error creating a fence event." });
        }

        if (FAILED(commandQueue->Signal(fence.Get(), 1)))
        {
            throw(std::runtime_error{ "Error siganalling buffer uploaded." });
        }

        if (FAILED(fence->SetEventOnCompletion(1, fenceEventHandle)))
        {
            throw(std::runtime_error{ "Failed set event on completion." });
        }
        // TODO this might be slowing down (if called more than initialization time) (I think this comment should be removed because in the tutorial it was called in ctor so here's the answer to that)
        DWORD wait{ WaitForSingleObject(fenceEventHandle, 10000) };
        if (wait != WAIT_OBJECT_0)
        {
            throw(std::runtime_error{ "Failed WaitForSingleObject()." });
        }

        return defaultBuffer;

    }


    template<typename T>
    Microsoft::WRL::ComPtr<ID3D12Resource> createStructuredBuffer(ID3D12Device* device, const std::vector<T>& data, std::wstring name = L"")
    {
        return createDefaultBuffer(device, data, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, name);
    }

    template<typename T>
    void createSrv(ID3D12Device* device, ID3D12DescriptorHeap* descHeap, int offset, ID3D12Resource* resource, size_t numElements)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Buffer.FirstElement = 0;
        srvDesc.Buffer.NumElements = static_cast<UINT>(numElements);
        srvDesc.Buffer.StructureByteStride = static_cast<UINT>(sizeof(T));
        srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

        static UINT descriptorSize{ device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) };
        D3D12_CPU_DESCRIPTOR_HANDLE d{ descHeap->GetCPUDescriptorHandleForHeapStart() };
        d.ptr += descriptorSize * offset;
        device->CreateShaderResourceView(resource, &srvDesc, d);
    }

    /**
     * \brief 
     * \tparam T Type of the data upload to the constant buffer [WARNING: IT SHOULD NOT EXCEED THE STANDARD SIZE LIMIT OF THE CONSTANT BUFFER)
     * \param name name of the constant buffer
     * \param swapChainBufferCount how many buffers do we have for our swap chain (by default 2)
     */
    template<typename T>
    ComPtr<ID3D12Resource> createConstantBuffer(const std::wstring& name, const int swapChainBufferCount = 2)
    {
        ComPtr<ID3D12Resource> constBuffer;
        UINT elementSizeAligned = (sizeof(T) + 255) & ~255; // constant buffer should be aligned with 256 bits
        UINT64 bufferSize{ elementSizeAligned * swapChainBufferCount};

        D3D12_HEAP_PROPERTIES heapProps;
        ZeroMemory(&heapProps, sizeof(heapProps));
        heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC resDesc;
        ZeroMemory(&resDesc, sizeof(resDesc));
        resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resDesc.Alignment = 0;
        resDesc.Width = bufferSize;
        resDesc.Height = 1;
        resDesc.DepthOrArraySize = 1;
        resDesc.MipLevels = 1;
        resDesc.Format = DXGI_FORMAT_UNKNOWN;
        resDesc.SampleDesc.Count = 1;
        resDesc.SampleDesc.Quality = 0;
        resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		HRESULT hr{ device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(constBuffer.ReleaseAndGetAddressOf())
		) };

        if (FAILED(hr))
        {
            throw(std::runtime_error{ "Error creating constant buffer." });
        }

        constBuffer->SetName(name.c_str());

        return constBUffer;
    }
}
