#include "App.h"

void REditor::App::Run()
{
	Init();
	engine->ProcessFrame();
	CleanUp();
}

void REditor::App::Init()
{
	engine = new Ruya::Engine();
	editorCamera = std::make_unique<EditorCamera>();
}

void REditor::App::CleanUp()
{
	delete engine;
}

