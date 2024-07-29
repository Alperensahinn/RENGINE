#pragma once

class RVulkan;

struct GLFWwindow;

class App
{
public:
	void Run();

private:
	void InitWindow();

	void InitRenderer();

	void MainLoop();

	void CleanUp();

private:
	GLFWwindow* glfwWindow;
	RVulkan* vulkan;
};