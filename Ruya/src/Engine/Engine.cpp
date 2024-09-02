#include "Engine.h"
#include "Utilities/FileSystem/FileSystem.h"

Ruya::Engine::Engine()
{
}

Ruya::Engine::~Engine()
{
	CleanUp();
}

Ruya::Renderer& Ruya::Engine::GetRenderer()
{
	return *pRenderer;
}

void Ruya::Engine::ProcessFrame()
{
	pRenderer->DrawFrame();
}

void Ruya::Engine::Init(RWindow* window)
{
	pRInput = new RInput(window->GetWindow());
	pRenderer = new Renderer(window->GetWindow());
	mainCamera = new Camera();
	pRenderer->BindCamera(mainCamera);
}

void Ruya::Engine::CleanUp()
{
	delete pRInput;
	delete pRenderer;
	delete mainCamera;
}
