#pragma once
#include "Graphics/Renderer/Renderer.h"
#include "Input/RInput.h"
#include "Scene/Camera.h"
#include "GameFramework/RGame.h"
#include "../Editor/App/Window/RWindow.h"

namespace Ruya
{
	class Engine
	{
	public:
		static Engine& GetInstance() 
		{
			static Engine instance;
			return instance;
		}

	private:
		Engine();
		~Engine();

		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;

	public:
		void Init(RWindow* window);
		void ProcessFrame();

		void SetMainCamera(Camera* camera);

		RWindow& GetWindow();
		Renderer& GetRenderer();

	private:
		void CleanUp();

	private:
		Renderer* pRenderer;
		RWindow* pWindow;
		Camera* mainCamera;
	};
}