#include "App.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

void App::Run()
{
	InitWindow();
	InitRenderer();
	MainLoop();
	CleanUp();
}

void App::InitWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindow = glfwCreateWindow(1600, 900, "Ruya", nullptr, nullptr);
}

void App::InitRenderer()
{
}

void App::MainLoop()
{
	while (!glfwWindowShouldClose(glfwWindow))
	{
		glfwPollEvents();
	}
}

void App::CleanUp()
{
	glfwDestroyWindow(glfwWindow);
	glfwTerminate();
}
