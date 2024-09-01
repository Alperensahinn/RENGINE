#include "Engine.h"

Ruya::Engine::Engine()
{
	Init();
}

Ruya::Engine::~Engine()
{
	CleanUp();
}

void Ruya::Engine::ProcessFrame()
{
	while (!pWindow->RWindowShouldClose())
	{
		pWindow->PoolEvents();
		pRenderer->DrawFrame();
	}
}

void Ruya::Engine::Init()
{
	pWindow = new RWindow();
	pRInput = new RInput(pWindow->GetWindow());
	pRenderer = new Renderer(pWindow->GetWindow());
	mainCamera = new Camera();
	pRenderer->BindCamera(mainCamera);
}

void Ruya::Engine::CleanUp()
{
	delete pWindow;
	delete pRInput;
	delete pRenderer;
	delete mainCamera;
}
