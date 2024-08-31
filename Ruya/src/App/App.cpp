#include "App.h"
#include "../Window/RWindow.h"
#include "../Graphics/Renderer/Renderer.h"
#include "../Input/RInput.h"
#include "../Scene/Camera.h"

#include <iostream>

namespace Ruya
{
	Camera* mainCamera;
	bool firstMouse = true;
	float lastX = 800, lastY = 450, pitch, yaw;

	void App::Run()
	{
		Init();
		MainLoop();
		CleanUp();
	}

	void App::Init()
	{
		mainCamera = new Camera();
		pWindow = new RWindow();
		pRenderer = new Renderer(pWindow->GetWindow());
		pRenderer->BindCamera(mainCamera);
		pRInput = new RInput(pWindow->GetWindow());

		glfwSetCursorPosCallback(&(pWindow->GetWindow()), MouseCallback);
	}

	void App::MainLoop()
	{
		while (!pWindow->RWindowShouldClose())
		{
			pWindow->PoolEvents();
			ProcessInput();
			pRenderer->DrawFrame();
		}
	}

	void App::CleanUp()
	{
		delete pWindow;
		delete pRenderer;
		delete pRInput;
	}

	void App::ProcessInput()
	{
		const float cameraSpeed = 0.05f;
		if (pRInput->GetKey(KeyCode::W))
		{
			mainCamera->transform.position += cameraSpeed * mainCamera->transform.front;
		}

		if (pRInput->GetKey(KeyCode::S))
		{
			mainCamera->transform.position -= cameraSpeed * mainCamera->transform.front;
		}

		if (pRInput->GetKey(KeyCode::A))
		{
			mainCamera->transform.position -= cameraSpeed * mainCamera->transform.GetRight();
		}

		if (pRInput->GetKey(KeyCode::D))
		{
			mainCamera->transform.position += cameraSpeed * mainCamera->transform.GetRight();
		}

		if (pRInput->GetKey(KeyCode::Q))
		{
			mainCamera->transform.position -= cameraSpeed * math::vec3(0.0f, 1.0f, 0.0f);
		}

		if (pRInput->GetKey(KeyCode::E))
		{
			mainCamera->transform.position += cameraSpeed * math::vec3(0.0f, 1.0f, 0.0f);
		}
	}

	void Ruya::MouseCallback(GLFWwindow* window, double xpos, double ypos)
	{
		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos;
		lastX = xpos;
		lastY = ypos;

		float sensitivity = 0.1f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;
		pitch += yoffset;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 direction;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		mainCamera->transform.front = glm::normalize(direction);
	}
}
