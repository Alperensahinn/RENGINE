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
			std::shared_ptr<RenderObject> renderObject = renderQueue->Pop();
			
			pRVulkan->Draw(pEngineUI, renderObject->mesh.meshBuffer, renderObject->material.material, camera->GetViewMatrix());
		}
	}

	RVulkan* Renderer::GetRendererBackend()
	{
		return pRVulkan;
	}

	RenderObject Renderer::CreateRenderObject(std::shared_ptr<Mesh> mesh, std::shared_ptr<Texture> texture)
	{	
		RenderObject renderObject;
		renderObject.mesh.meshBuffer = rvkLoadMesh(pRVulkan, mesh->vertices, mesh->indices);

		RVkMetallicRoughness::MaterialResources materialResources = {};
		materialResources.albedoImage = texture->image;
		materialResources.albedoSampler = pRVulkan->defaultSamplerNearest;

		renderObject.material.material = pRVulkan->metallicRoughnessPipeline.WriteMaterial(pRVulkan, MaterialPass::MainColor, materialResources, pRVulkan->globalDescriptorAllocator);

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
