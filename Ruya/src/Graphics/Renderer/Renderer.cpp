#include "Renderer.h"
#include "../DefaultObjects/DefaultCube.h"

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
		pRVulkan->Draw(pEngineUI, geometry);
	}

	RVulkan* Renderer::GetRendererBackend()
	{
		return pRVulkan;
	}

	void Renderer::LoadMesh()
	{
		DefaultCube* defaultCube = new DefaultCube();
		auto vertices = defaultCube->GetVertices();
		auto indices = defaultCube->GetIndices();

		geometry = rvkLoadMesh(pRVulkan, vertices, indices);

		pRVulkan->deletionQueue.PushFunction([&]() 
			{
			rvkDestoryBuffer(pRVulkan, geometry.vertexBuffer);
			rvkDestoryBuffer(pRVulkan, geometry.indexBuffer);
			});
	}

	void Renderer::Init(GLFWwindow& window)
	{
		pRVulkan = new RVulkan(window);
		pEngineUI = new EngineUI(window, this);
		LoadMesh();
	}

	void Renderer::CleanUp()
	{
		pRVulkan->WaitDeviceForCleanUp();

		deletionQueue.flush();
		delete pEngineUI;
		delete pRVulkan;
	}
}
