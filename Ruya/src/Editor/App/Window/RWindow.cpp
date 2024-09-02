#include "RWindow.h"
#include "../../../Engine/Utilities/Log/RLog.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


RWindow::RWindow()
{
	InitGLFWWindow();
}

RWindow::~RWindow()
{
	CleanUp();
}

GLFWwindow& RWindow::GetWindow()
{
	return *glfwWindow;
}

bool RWindow::RWindowShouldClose()
{
	return glfwWindowShouldClose(glfwWindow);
}

void RWindow::PoolEvents()
{
	glfwPollEvents();
}

void RWindow::InitGLFWWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindow = glfwCreateWindow(1600, 900, "Ruya", nullptr, nullptr);

	unsigned int requiredExtensionsCount;
	const char** requiredExtensions = glfwGetRequiredInstanceExtensions(&requiredExtensionsCount);

	RLOG("[GLFW] Vulkan required extension:");

	for (unsigned int i = 0; i < requiredExtensionsCount; ++i)
	{
		std::cout << "\t" << requiredExtensions[i] << std::endl;
	}
}

void RWindow::CleanUp()
{
	glfwDestroyWindow(glfwWindow);
}

