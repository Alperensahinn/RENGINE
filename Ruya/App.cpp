#include "App.h"

#include "RVulkan.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

void App::Run()
{
	InitWindow();
	InitRenderer();
	MainLoop();
	CleanUp();
}

GLFWwindow& App::GetWindow()
{
	return *glfwWindow;
}

void App::InitWindow()
{
	//init glfw
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindow = glfwCreateWindow(1600, 900, "Ruya", nullptr, nullptr);

	unsigned int requiredExtensionsCount;
	const char** requiredExtensions = glfwGetRequiredInstanceExtensions(&requiredExtensionsCount);
	
	std::cout << "[GLFW] Vulkan required extension:" << std::endl;

	for (unsigned int i = 0; i < requiredExtensionsCount; ++i) 
	{
		std::cout << "\t" << requiredExtensions[i] << std::endl;
	}
}

void App::InitRenderer()
{
	//init vulkan
	vulkan = new RVulkan(GetWindow());
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
