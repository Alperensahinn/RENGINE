#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "../../Engine/Engine.h"
#include "../../Editor/Camera/EditorCamera.h"
#include <memory>


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
		std::unique_ptr<EditorCamera> editorCamera;
	};
}