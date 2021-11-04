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
	assert(!m_contentLoaded && "Content should be unloaded before destroying game object");
	assert(m_window.use_count() == 1 && "There should be only m_window pointing to the window object");
	Application::Get().DestroyWindow(m_window->getName());
	m_window.reset();
}


bool Game::Initialize()
{
	if (DirectX::XMVerifyCPUSupport() == false)
	{
		MessageBoxA(NULL, "Failed to verify DirectX math support", "Error", MB_OK | MB_ICONERROR);
		return false;
	}

	m_window = Application::Get().createRenderWindow(m_name, m_width, m_height);

	//m_window = Application::Get().createRenderWindow();
	m_window->SetGamePtr(shared_from_this());
	m_window->ShowWindow();

	return true;
}



void Game::Reset()
{
	Mizu::Application::Get().DestroyWindow(m_window->getName());
}


void Game::OnResize(Mizu::ReizeEventArgs& e)
{
	m_width = e.width;
	m_height = e.height;
}

void Game::OnWindowDestroy()
{
	UnloadContent();
}