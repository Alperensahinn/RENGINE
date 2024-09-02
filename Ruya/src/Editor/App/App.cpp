#include "App.h"
#include "../../Game/ProjectB.h"

void REditor::App::Run()
{
	Init();
	game->Start();

	while (!pWindow->RWindowShouldClose())
	{
		pWindow->PoolEvents();
		game->Update();
		Ruya::Engine::GetInstance().ProcessFrame();
	}

	CleanUp();
}

void REditor::App::Init()
{
	pWindow = new RWindow();
	Ruya::Engine::GetInstance().Init(pWindow);
	editorCamera = std::make_unique<EditorCamera>();


	game = std::make_unique<ProjectB>();
}

void REditor::App::CleanUp()
{

}

