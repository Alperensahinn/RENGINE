#pragma once
#include "Window/RWindow.h"
#include "Graphics/Renderer/Renderer.h"
#include "Input/RInput.h"
#include "Scene/Camera.h"
#include "GameFramework/RGame.h"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace Ruya
{
	enum EngineMode
	{
		Editor,
		Game
	};

	class Engine
	{
	public:
		Engine();
		~Engine();

		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;

	public:
		void ProcessFrame();

		EngineMode GetEngineMode();
		void SetEngineMode(EngineMode engineMode);

	private:
		void Init();
		void CleanUp();

	private:
		RWindow* pWindow;
		Renderer* pRenderer;
		RInput* pRInput;
		Camera* mainCamera;

		EngineMode mode;
	};
}