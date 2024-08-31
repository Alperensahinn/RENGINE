#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Ruya
{
	enum  KeyCode
	{
		A,
		D,
		E,
		Q,
		S,
		W
	};

	class RInput
	{
	public:

	public:
		RInput(GLFWwindow& window);
		~RInput();

		RInput(const RInput&) = delete;
		RInput& operator=(const RInput&) = delete;

	public:
		bool GetKey(KeyCode keyCode);

	private:
		GLFWwindow& window;
	};
}