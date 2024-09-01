#pragma once
#include "Window/RWindow.h"
#include "Graphics/Renderer/Renderer.h"
#include "Input/RInput.h"
#include "Scene/Camera.h"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace Ruya
{
	class Engine
	{
	public:
		Engine();
		~Engine();

		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;

	public:
		void ProcessFrame();

	private:
		void Init();
		void CleanUp();

	private:
		RWindow* pWindow;
		Renderer* pRenderer;
		RInput* pRInput;
		Camera* mainCamera;
	};
}