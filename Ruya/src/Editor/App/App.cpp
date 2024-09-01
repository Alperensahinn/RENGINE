#include "App.h"

void Ruya::Editor::App::Run()
{
	Init();
	engine->ProcessFrame();
	CleanUp();
}

void Ruya::Editor::App::Init()
{
	engine = new Engine();
}

void Ruya::Editor::App::CleanUp()
{
	delete engine;
}

