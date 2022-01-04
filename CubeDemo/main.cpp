#include "CubeDemo.hpp"
#include "Mizu/Application.hpp"

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{

	Mizu::Application::Create(hInstance);
	auto demo = std::make_shared<Mizu::CubeDemo>(1280, 720, false);
	Mizu::Application& app = Mizu::Application::Get();

	auto ret = app.Run(demo);

	// TODO find a better way to handle that close handle
	app.Close();

	return 0;
}