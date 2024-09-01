#pragma once
#include "../../Engine/Engine.h"
#include "../Camera/EditorCamera.h"

namespace REditor
{
	class App
	{
	public:
		void Run();

	private:
		void Init();
		void CleanUp();

	private:
		Ruya::Engine* engine;
		std::unique_ptr<EditorCamera> editorCamera;
	};
}