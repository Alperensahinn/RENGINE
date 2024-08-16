#include "App.h"
#include "../Window/RWindow.h"
#include "../Graphics/Renderer/Renderer.h"

#include <iostream>

void App::Run()
{
	Init();
	MainLoop();
	CleanUp();
}

void App::Init()
{
	pWindow = new RWindow();
	pRenderer = new Renderer(pWindow->GetWindow());
}

void App::MainLoop()
{
	while (!pWindow->RWindowShouldClose())
	{
		pWindow->PoolEvents();
		pRenderer->DrawFrame();
	}
}

void App::CleanUp()
{
	delete pWindow;
	delete pRenderer;
}
