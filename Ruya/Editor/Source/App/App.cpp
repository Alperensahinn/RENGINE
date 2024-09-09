#include "App.h"
#include <ProjectB.h>

void REditor::App::Run()
{
	Init();
	game->Start();

	while (!pWindow->RWindowShouldClose())
	{
		pWindow->PoolEvents();
		editorCamera->Update();
		game->Update();
		Ruya::Engine::GetInstance().ProcessFrame();
	}

	CleanUp();
}

void REditor::App::Init()
{
	pWindow = new RWindow();
	
	editorCamera = new EditorCamera();

	Ruya::Engine::GetInstance().Init(pWindow);
	Ruya::Engine::GetInstance().SetMainCamera(editorCamera);

	game = std::make_unique<ProjectB>();
}

void REditor::App::CleanUp()
{
	game->CleanUp();
}

