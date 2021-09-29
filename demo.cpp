#include "CubeDemo.hpp"
#include "Application.hpp"

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
	Mizu::CubeDemo demo(1280, 720,false);

	demo.create(hInstance);
	demo.LoadContent();

	//Mizu::Application::Create(hInstance);
	Mizu::Application& app = Mizu::Application::Get();


	// to print for example
	BOOL bRet;
	MSG msg;
	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (bRet == -1)
		{
			OutputDebugStringW(L"ERROR\n");
			break;
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	app.Flush();

	// TODO find a better way to handle that close handle
	app.Close();

	return 0;
}