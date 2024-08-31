#include "RInput.h"

Ruya::RInput::RInput(GLFWwindow& window) : window(window)
{
}

Ruya::RInput::~RInput()
{
}

bool Ruya::RInput::GetKeyDown(KeyCode keyCode)
{
	switch (keyCode)
	{
	case RInput::A:
		return glfwGetKey(&window, GLFW_KEY_A) == GLFW_PRESS;
		break;
	case RInput::D:
		return glfwGetKey(&window, GLFW_KEY_D) == GLFW_PRESS;
		break;
	case RInput::S:
		return glfwGetKey(&window, GLFW_KEY_S) == GLFW_PRESS;
		break;
	case RInput::W:
		return glfwGetKey(&window, GLFW_KEY_W) == GLFW_PRESS;
		break;
	default:
		break;
	}
}
