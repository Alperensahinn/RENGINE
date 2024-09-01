#pragma once
#include "../Vulkan/RVulkan.h"
#include "../../EngineUI/EngineUI.h"
#include "../../Collections/RDeletionQueue.h"

struct GLFWwindow;

namespace Ruya
{
	struct RVulkan;
	class Camera;

	class Renderer
	{
	public:
		Renderer(GLFWwindow& window);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

	public:
		void DrawFrame();
		RVulkan* GetRendererBackend();

		void LoadMesh();
		void BindCamera(Camera* camera);

	private:
		void Init(GLFWwindow& window);
		void CleanUp();

	public:
		RDeletionQueue deletionQueue;

	private:
		RVulkan* pRVulkan;
		EngineUI* pEngineUI;

		//test
		RVkMeshBuffer geometry;

		Camera* camera;
	};
}