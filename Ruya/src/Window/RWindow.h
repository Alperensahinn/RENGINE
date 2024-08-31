#pragma once

struct GLFWwindow;

namespace Ruya
{
	class RWindow
	{
	public:
		RWindow(unsigned int windowWidth, unsigned int windowHeight);
		~RWindow();

		RWindow(const RWindow&) = delete;
		RWindow& operator=(const RWindow&) = delete;

	public:
		GLFWwindow& GetWindow();
		bool RWindowShouldClose();
		void PoolEvents();

	private:
		void InitGLFWWindow(unsigned int windowWidth, unsigned int windowHeight);
		void CleanUp();

	private:
		GLFWwindow* glfwWindow;
	};
}