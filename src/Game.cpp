#include "Game.hpp"
#include "Window.hpp"
#include "Application.hpp"

using Mizu::Game;



Game::Game(const std::wstring& name, int width, int height, bool vSync) :
	m_width(width), m_height(height), m_vsync(vSync), m_name(name)
{

}


Game::~Game()
{
	// TODO
}


bool Game::Initialize()
{
	if (DirectX::XMVerifyCPUSupport() == false)
	{
		MessageBoxA(NULL, "Failed to verify DirectX math support", "Error", MB_OK | MB_ICONERROR);
		return false;
	}
	// TODO create window
	return true;
}



void Game::Reset()
{
	Mizu::Application::Get().DestroyWindow(m_window);
	m_window.reset();

}


void Game::OnResize(Mizu::ReizeEventArgs& e)
{
	m_width = e.width;
	m_height = e.height;
}

void Game::OnWindowDestroy()
{

}