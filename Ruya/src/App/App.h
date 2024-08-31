#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace Ruya
{
	class RWindow;
	class Renderer;
	class RInput;

	class App
	{
	public:
		void Run();

	private:
		void Init();

		void MainLoop();

		void CleanUp();

		void ProcessInput();

	private:
		RWindow* pWindow;
		Renderer* pRenderer;
		RInput* pRInput;
	};

	void MouseCallback(GLFWwindow* window, double xpos, double ypos);
}