#pragma once
#include "Graphics/Renderer/Renderer.h"
#include "Input/RInput.h"
#include "Scene/Camera.h"
#include "GameFramework/RGame.h"
#include "Window/RWindow.h"
#include "../../Editor/Source/UI/Panel.h"

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
		void SetEditorPanels(std::vector<REditor::Panel*>& panels);
		std::vector<REditor::Panel*>& GetEditorPanels();

		RWindow& GetWindow();
		Renderer& GetRenderer();

	private:
		void CleanUp();

	private:
		Renderer* pRenderer;
		RWindow* pWindow;
		Camera* mainCamera;

		std::vector <REditor::Panel*> editorPanels;
	};
}