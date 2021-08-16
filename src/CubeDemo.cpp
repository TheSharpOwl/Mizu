#include "..\inc\CubeDemo.hpp"
#include <Application.hpp>

using namespace Mizu;

CubeDemo::CubeDemo(int width, int height, bool vsync) :
	m_width(width), m_height(height), m_vsync(vsync)
{

}

bool CubeDemo::create(HINSTANCE hInst)
{
	if (DirectX::XMVerifyCPUSupport())
	{
		MessageBoxA(NULL, "Failed to verify DirectX math support", "Error", MB_OK | MB_ICONERROR);
		return false;
	}

	// might need to put a pointer to this demo in the window later (for controls of mouse and keyboard for example)
	Application::Create(hInst);

	return true;
}

bool Mizu::CubeDemo::LoadContent()
{
	auto device = Application::Get().GetDevice();
	auto commandQueue = Application::Get().GetCommandQueue();
	// TODO next time : make the command lists 3 because now we need copy one and the one we have already is only direct 
	auto commandList = Application::Get().GetCommandList();
	return false;
}

