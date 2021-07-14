#include "Game.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <DirectXMath.h>

bool Game::Initialize()
{
	// check for direct3d math support
	if (!DirectX::XMVerifyCPUSupport())
	{
		MessageBoxA(NULL, "Failed to verify DirectX math lib", ERROR, MB_OK | MB_ICONERROR);
	}


}

void Game::Destroy()
{

}

void Game::OnUpdate(UpdateEventArgs& e) {}

void Game::OnRender(RenderEventArgs& e) {}

void Game::OnKeyPressed(KeyEventArgs& e) {}

void Game::OnKeyReleased(KeyEventArgs& e) {}

void Game::OnMouseMoved(MouseMotionEventArgs& e) {}

void Game::OnMouseButtonPressed(MouseButtonEventArgs& e) {}

void Game::OnMouseButtonReleased(MouseButtonEventArgs& e) {}

void Game::OnMouseWheel(MouseWheelEventArgs& e) {}

void Game::OnResize(ResizeEventArgs& e) {}

void Game::OnWindowDestroy() {}