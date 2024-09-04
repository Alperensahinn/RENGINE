#pragma once

struct GLFWwindow;

class RWindow
{
public:
	RWindow();
	~RWindow();

	RWindow(const RWindow&) = delete;
	RWindow& operator=(const RWindow&) = delete;

public:
	GLFWwindow& GetWindow();
	bool RWindowShouldClose();
	void PoolEvents();

private:
	void InitGLFWWindow();
	void CleanUp();

private:
	GLFWwindow* glfwWindow;
};
