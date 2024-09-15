#include "Renderer.h"
#include "../../Utilities/FileSystem/FileSystem.h"
#include "../Mesh.h"
#include "../../Scene/Camera.h"
#include "RenderQueue.h"
#include <memory>

#include "RenderObject.h"
#include "Texture.h"

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

		pRVulkan->BeginFrame();

		pRVulkan->BeginDraw();

		while(!renderQueue->IsEmpty())
		{
			std::shared_ptr<RenderObject> renderObject = renderQueue->Pop();
			
			pRVulkan->Draw(renderObject->meshBuffer, renderObject->material, renderObject->modelMatrix, camera->GetViewMatrix(), camera->transform.position);
		}

		pRVulkan->EndDraw();

		pRVulkan->LightPass();

		pRVulkan->DrawEngineUI(pEngineUI);

		pRVulkan->EndFrame();
	}

	RVulkan* Renderer::GetRendererBackend()
	{
		return pRVulkan;
	}

	RenderObject Renderer::CreateRenderObject(std::shared_ptr<Mesh> mesh, Texture albedoTexture, Texture normalTexture)
	{	
		RenderObject renderObject;
		renderObject.meshBuffer = rvkCreateMeshBuffer(pRVulkan, mesh->vertices, mesh->indices);
		
		PBRMaterial material;
		material.resources.albedoTexture = albedoTexture;
		material.resources.normalTexture = normalTexture;

		material.Init(material.resources);

		renderObject.material = material;

		return renderObject;
	}

	void Renderer::BindCamera(Camera* camera)
	{
		this->camera = camera;
	}

	void Renderer::AddToRenderQueue(std::shared_ptr<RenderObject> renderObject)
	{
		renderQueue->Push(renderObject);
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
