#include "App.h"

#include "RVulkan.h"

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
	//init glfw
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindow = glfwCreateWindow(1600, 900, "Ruya", nullptr, nullptr);
}

void App::InitRenderer()
{
	//init vulkan
	vulkan = new RVulkan();
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
	//destroy glfw
	glfwDestroyWindow(glfwWindow);
	glfwTerminate();

	//destroy vulkan
	delete vulkan;
}
