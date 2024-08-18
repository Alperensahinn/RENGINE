#pragma once

namespace Ruya
{
	class RWindow;
	class Renderer;

	class App
	{
	public:
		void Run();

	private:
		void Init();

		void MainLoop();

		void CleanUp();

	private:
		RWindow* pWindow;
		Renderer* pRenderer;
	};
}