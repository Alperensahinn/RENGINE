#include "App.h"
#include "../Window/RWindow.h"
#include "../Graphics/Renderer/Renderer.h"
#include "../Input/RInput.h"
#include "../Scene/Camera.h"

#include <iostream>

namespace Ruya
{
	void App::Run()
	{
		Init();
		MainLoop();
		CleanUp();
	}

	void App::Init()
	{
		camera = new Camera();
		pWindow = new RWindow();
		pRenderer = new Renderer(pWindow->GetWindow());
		pRenderer->BindCamera(camera);
		pRInput = new RInput(pWindow->GetWindow());
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
		if (pRInput->GetKeyDown(RInput::KeyCode::W))
		{
			camera->transform.position -= cameraSpeed * camera->transform.front;
		}

		if (pRInput->GetKeyDown(RInput::KeyCode::S))
		{
			camera->transform.position += cameraSpeed * camera->transform.front;
		}

		if (pRInput->GetKeyDown(RInput::KeyCode::A))
		{
			camera->transform.position -= cameraSpeed * camera->transform.right;
		}

		if (pRInput->GetKeyDown(RInput::KeyCode::D))
		{
			camera->transform.position += cameraSpeed * camera->transform.right;
		}
	}
}
