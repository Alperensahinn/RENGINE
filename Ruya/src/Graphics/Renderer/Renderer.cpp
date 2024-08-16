#include "Renderer.h"
#include "../Vulkan/RVulkan.h"

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
}

void Renderer::CleanUp()
{
	delete pRVulkan;
}
