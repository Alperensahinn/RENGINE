#pragma once

namespace Ruya
{
	class RWindow;
	class Renderer;
	class RInput;
	class Camera;

	class App
	{
	public:
		void Run();

	private:
		void Init();

		void MainLoop();

		void CleanUp();

		void ProcessInput();

	private:
		RWindow* pWindow;
		Renderer* pRenderer;
		RInput* pRInput;
		Camera* camera;
	};
}