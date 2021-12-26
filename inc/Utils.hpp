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

        auto temp = resources_dir + std::wstring(filename);
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
}