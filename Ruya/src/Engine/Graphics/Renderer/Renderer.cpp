#include "Renderer.h"
#include "../DefaultObjects/DefaultCube.h"
#include "../../Utilities/FileSystem/FileSystem.h"
#include "../Mesh.h"
#include "../../Scene/Camera.h"
#include "RenderQueue.h"
#include "Drawable.h"
#include <memory>

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
		if(pRVulkan->resizeRequest)
		{
			pRVulkan->ResizeSwapChain();
		}

		while(!renderQueue->IsEmpty())
		{
			std::shared_ptr<Drawable> mesh = renderQueue->Pop();

			pRVulkan->Draw(pEngineUI, mesh->meshBuffer, camera->GetViewMatrix());
		}
	}

	RVulkan* Renderer::GetRendererBackend()
	{
		return pRVulkan;
	}

	std::shared_ptr<Drawable> Renderer::LoadMesh(std::shared_ptr<Mesh> mesh)
	{	
		auto vertices = mesh->vertices;
		auto indices = mesh->indices;

		std::shared_ptr<Drawable> drawable = std::make_shared<Drawable>();

		drawable->meshBuffer = rvkLoadMesh(pRVulkan, vertices, indices);

		pRVulkan->deletionQueue.PushFunction([=]() 
			{
			rvkDestoryBuffer(pRVulkan, drawable->meshBuffer.vertexBuffer);
			rvkDestoryBuffer(pRVulkan, drawable->meshBuffer.indexBuffer);
			});

		return drawable;
	}

	void Renderer::BindCamera(Camera* camera)
	{
		this->camera = camera;
	}

	void Renderer::AddToRenderQueue(std::shared_ptr<Drawable> mesh)
	{
		renderQueue->Push(mesh);
	}

	void Renderer::Init(GLFWwindow& window)
	{
		pRVulkan = new RVulkan(window);
		pEngineUI = new EngineUI(window, this);
		renderQueue = new RenderQueue();
	}

	void Renderer::CleanUp()
	{
		pRVulkan->WaitDeviceForCleanUp();

		deletionQueue.flush();
		delete pEngineUI;
		delete pRVulkan;
		delete renderQueue;
	}
}
