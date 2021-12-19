#include<string>

namespace Mizu
{
    //std::wstring to_wstring(LPCWSTR s)
    //{
    //    return L"";// delete todo
    //}

    std::wstring to_wstring(LPCSTR s)
    {
        std::string temp(s);
        std::wstring ans(temp.begin(), temp.end());
        return ans;
    }

	void CompileShader(LPCWSTR filename, LPCSTR entryPoint, LPCSTR targetType, UINT compileFlags, ComPtr<ID3DBlob> shaderBlob)
	{
        ID3DBlob* errorMessages;
        HRESULT hr = D3DCompileFromFile(filename, nullptr, nullptr, entryPoint, targetType, compileFlags, 0, &shaderBlob, &errorMessages);

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
}