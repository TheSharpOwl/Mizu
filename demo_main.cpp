#include "CubeDemo.hpp"
#include "Application.hpp"

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{

	Mizu::Application::Create(hInstance);
	auto demo = std::make_shared<Mizu::CubeDemo>(1280, 720,false);
	Mizu::Application& app = Mizu::Application::Get();
	demo->Initialize();
	demo->LoadContent();


	{
		//std::shared_ptr<Mizu::Game> demo = std::make_shared()
	}

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