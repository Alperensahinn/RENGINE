#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <Engine.h>
#include <GameFramework/RGame.h>
#include "../../Source/Camera/EditorCamera.h"
#include <memory>
#include <Window/RWindow.h>


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
		RWindow* pWindow;
		std::unique_ptr<Ruya::RGame> game;
		EditorCamera* editorCamera;
	};
}