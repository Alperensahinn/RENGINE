#include "Renderer.h"
#include "../DefaultObjects/DefaultCube.h"
#include "../../Utilities/FileSystem/FileSystem.h"
#include "../../Scene/Model.h"
#include "../../Scene/Camera.h"
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
		pRVulkan->Draw(pEngineUI, geometry, camera->GetViewMatrix());
	}

	RVulkan* Renderer::GetRendererBackend()
	{
		return pRVulkan;
	}

	void Renderer::LoadMesh()
	{	
		std::shared_ptr<Mesh> mesh;
		mesh = ImportFBXMesh("C:\\Users\\aalpe\\Desktop\\RENGINE\\Ruya\\src\\TestMeshes\\Monkey.glb");

		auto vertices = mesh->vertices;
		auto indices = mesh->indices;


		geometry = rvkLoadMesh(pRVulkan, vertices, indices);

		pRVulkan->deletionQueue.PushFunction([=]() 
			{
			rvkDestoryBuffer(pRVulkan, geometry.vertexBuffer);
			rvkDestoryBuffer(pRVulkan, geometry.indexBuffer);
			});
	}

	void Renderer::BindCamera(Camera* camera)
	{
		this->camera = camera;
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
