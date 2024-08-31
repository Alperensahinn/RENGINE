#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Ruya
{
	class RInput
	{
	public:
		enum  KeyCode
		{
			A,
			D,
			S,
			W
		};

	public:
		RInput(GLFWwindow& window);
		~RInput();

		RInput(const RInput&) = delete;
		RInput& operator=(const RInput&) = delete;

	public:
		bool GetKeyDown(KeyCode keyCode);

	private:
		GLFWwindow& window;
	};
}