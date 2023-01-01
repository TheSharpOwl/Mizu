#include "Game.hpp"
#include "Application.hpp"
#include "Window.hpp"

using Mizu::Game;

Game::Game(const std::wstring& name, int width, int height, bool vSync)
    : m_width(width)
    , m_height(height)
    , m_vsync(vSync)
    , m_name(name)
{
}

Game::~Game()
{
    assert(!m_contentLoaded && "Content should be unloaded before destroying game object");
    assert(m_window.use_count() == 1 && "There should be only m_window pointing to the window object");
    m_window.reset();
}

bool Game::initialize()
{
    if (DirectX::XMVerifyCPUSupport() == false)
    {
        MessageBoxA(NULL, "Failed to verify DirectX math support", "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    m_window = Application::get().createRenderWindow(m_name, m_width, m_height);

    // m_window = Application::get().createRenderWindow();
    m_window->SetGamePtr(shared_from_this());
    m_window->ShowWindow();

    return true;
}

void Game::reset()
{
    Mizu::Application::get().destroyWindow(m_window->getName());
}

void Game::onKeyPressed(KeyEventArgs& e) { }

void Game::onKeyReleased(KeyEventArgs& e) { }

void Game::onMouseButtonPressed(MouseButtonEventArgs& e) { }

void Game::onMouseButtonReleased(MouseButtonEventArgs& e) { }

void Game::onMouseMoved(MouseMotionEventArgs& e) { }

void Game::onMouseWheel(MouseWheelEventArgs& e) { }

void Game::onResize(Mizu::ResizeEventArgs& e)
{
    m_width = e.width;
    m_height = e.height;
}
// TODO use this later instead of manually doing it
void Game::onWindowDestroy()
{
    unloadContent();
}