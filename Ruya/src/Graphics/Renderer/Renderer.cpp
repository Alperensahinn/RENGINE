#include "Renderer.h"

namespace Ruya 
{
	Renderer::Renderer(GLFWwindow& window)
	{
		Init(window);
	}

	Renderer::~Renderer()
	{
		CleanUp();
	}

	void Renderer::DrawFrame()
	{
		pEngineUI->Draw();
		pRVulkan->Draw(pEngineUI);
	}

	RVulkan* Renderer::GetRendererBackend()
	{
		return pRVulkan;
	}

	void Renderer::Init(GLFWwindow& window)
	{
		pRVulkan = new RVulkan(window);
		pEngineUI = new EngineUI(window, this);
	}

	void Renderer::CleanUp()
	{
		deletionQueue.flush();
		delete pEngineUI;
		delete pRVulkan;
	}
}
