#include "Renderer.h"
#include "../Vulkan/RVulkan.h"

namespace Ruya 
{
	Renderer::Renderer(GLFWwindow& window)
	{
		pRVulkan = new RVulkan(window);
	}

	Renderer::~Renderer()
	{
		CleanUp();
	}

	void Renderer::DrawFrame()
	{
		pRVulkan->Draw();
	}

	void Renderer::CleanUp()
	{
		delete pRVulkan;
	}
}
