#include "RInput.h"

Ruya::RInput::RInput(GLFWwindow& window) : window(window)
{
}

Ruya::RInput::~RInput()
{
}

bool Ruya::RInput::GetKey(KeyCode keyCode)
{
	if(keyCode == KeyCode::A)
		return glfwGetKey(&window, GLFW_KEY_A) == GLFW_PRESS;

	if (keyCode == KeyCode::D)
		return glfwGetKey(&window, GLFW_KEY_D) == GLFW_PRESS;

	if (keyCode == KeyCode::E)
		return glfwGetKey(&window, GLFW_KEY_E) == GLFW_PRESS;

	if (keyCode == KeyCode::Q)
		return glfwGetKey(&window, GLFW_KEY_Q) == GLFW_PRESS;

	if (keyCode == KeyCode::S)
		return glfwGetKey(&window, GLFW_KEY_S) == GLFW_PRESS;

	if (keyCode == KeyCode::W)
		return glfwGetKey(&window, GLFW_KEY_W) == GLFW_PRESS;
}
