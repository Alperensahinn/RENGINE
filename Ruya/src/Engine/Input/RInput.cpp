#include "RInput.h"
#include "../Engine.h"
#include "../Utilities/Log/RLog.h"

float Ruya::RInput::firstMouse = true;
float Ruya::RInput::mouseLastX = 0.0f;
float Ruya::RInput::mouseLastY = 0.0f;
float Ruya::RInput::mouse_offset_x = 0.0f;
float Ruya::RInput::mouse_offset_y = 0.0f;
float Ruya::RInput::mouse_offset_x_last = 0.0f;
float Ruya::RInput::mouse_offset_y_last = 0.0f;

Ruya::RInput::RInput()
{
}

Ruya::RInput::~RInput()
{
}

void Ruya::RInput::Init()
{
}

bool Ruya::RInput::GetKey(KeyCode keyCode)
{
	if(keyCode == KeyCode::A)
		return glfwGetKey(&Engine::GetInstance().GetWindow().GetWindow(), GLFW_KEY_A) == GLFW_PRESS;

	if (keyCode == KeyCode::D)
		return glfwGetKey(&Engine::GetInstance().GetWindow().GetWindow(), GLFW_KEY_D) == GLFW_PRESS;

	if (keyCode == KeyCode::E)
		return glfwGetKey(&Engine::GetInstance().GetWindow().GetWindow(), GLFW_KEY_E) == GLFW_PRESS;

	if (keyCode == KeyCode::Q)
		return glfwGetKey(&Engine::GetInstance().GetWindow().GetWindow(), GLFW_KEY_Q) == GLFW_PRESS;

	if (keyCode == KeyCode::S)
		return glfwGetKey(&Engine::GetInstance().GetWindow().GetWindow(), GLFW_KEY_S) == GLFW_PRESS;

	if (keyCode == KeyCode::W)
		return glfwGetKey(&Engine::GetInstance().GetWindow().GetWindow(), GLFW_KEY_W) == GLFW_PRESS;

	return false;
}

bool Ruya::RInput::GetMouseButton(MouseButton mouseButton)
{
	if(mouseButton == MouseButton::RIGHT)
		return glfwGetMouseButton(&Engine::GetInstance().GetWindow().GetWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

	if (mouseButton == MouseButton::LEFT)
		return glfwGetMouseButton(&Engine::GetInstance().GetWindow().GetWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

}

Ruya::math::vec2 Ruya::RInput::GetMouseDelta()
{
	double xpos;
	double ypos;

	glfwGetCursorPos(&Engine::GetInstance().GetWindow().GetWindow(), &xpos, &ypos);

	double deltaX = xpos - mouseLastX;
	double deltaY = mouseLastY - ypos;

	mouseLastX = xpos;
	mouseLastY = ypos;

	return math::vec2((float)deltaX, (float)deltaY);
}

void Ruya::RInput::SetCursorEnabled(bool isEnabled)
{
	if(isEnabled)
		glfwSetInputMode(&Engine::GetInstance().GetWindow().GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	else
		glfwSetInputMode(&Engine::GetInstance().GetWindow().GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}
