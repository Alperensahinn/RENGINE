#include "App.h"
#include <ProjectB.h>
#include <UI/DockSpace.h>
#include <UI/MainViewport.h>
#include <UI/MenuBar.h>
#include <UI/ActorInspector.h>
#include <UI/SceneOutliner.h>
#include <UI/LeftSideBar.h>

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

	game = std::make_shared<ProjectB>();
	Ruya::Engine::GetInstance().SetGame(game);

	editorPanels.push_back(new DockSpace());
	DockSpace* dockSpace = dynamic_cast<REditor::DockSpace*>(editorPanels.back());
	dockSpace->childPanels.push_back(new MenuBar());
	dockSpace->childPanels.push_back(new LeftSideBar());
	dockSpace->childPanels.push_back(new MainViewport());
	dockSpace->childPanels.push_back(new ActorInspector());
	dockSpace->childPanels.push_back(new SceneOutliner());

	Ruya::Engine::GetInstance().SetEditorPanels(editorPanels);
}

void REditor::App::CleanUp()
{
	Ruya::Engine::GetInstance().GetRenderer().pRVulkan->WaitDeviceForCleanUp();
	game->CleanUp();

	for (auto panel : editorPanels) {
		delete panel;
	}
}

