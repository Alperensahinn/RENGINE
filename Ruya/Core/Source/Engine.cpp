#include "Engine.h"
#include "Utilities/FileSystem/FileSystem.h"
#include "Core/Time.h"

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
	Time::UpdateTime();
	pRenderer->DrawFrame();
}

void Ruya::Engine::SetMainCamera(Camera* camera)
{
	mainCamera = camera;
	pRenderer->BindCamera(mainCamera);
}

RWindow& Ruya::Engine::GetWindow()
{
	return *pWindow;
}

void Ruya::Engine::Init(RWindow* window)
{
	pWindow = window;
	pRenderer = new Renderer(pWindow->GetWindow());
	mainCamera = new Camera();

	RInput::Init();
}

void Ruya::Engine::CleanUp()
{
	delete pRenderer;
	delete pWindow;
	delete mainCamera;
}
