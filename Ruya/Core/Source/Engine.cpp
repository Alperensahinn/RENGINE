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

void Ruya::Engine::SetGame(std::shared_ptr<RGame> game)
{
	this->game = game;
}

std::shared_ptr<Ruya::RGame> Ruya::Engine::GetGame()
{
	return game;
}

void Ruya::Engine::SetMainCamera(Camera* camera)
{
	mainCamera = camera;
	pRenderer->BindCamera(mainCamera);
}

void Ruya::Engine::SetEditorPanels(std::vector<REditor::Panel*>& panels)
{
	editorPanels = panels;
}

std::vector<REditor::Panel*>& Ruya::Engine::GetEditorPanels()
{
	return editorPanels;
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
