#pragma once
#include "../Vulkan/RVulkan.h"
#include "../../EngineUI/EngineUI.h"
#include "../../Collections/RDeletionQueue.h"

struct GLFWwindow;

namespace Ruya
{
	class RVulkan;
	class Camera;
	class RenderQueue;
	struct IDrawable;

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

		std::shared_ptr<IDrawable> LoadMesh(std::shared_ptr<Mesh> mesh);
		void BindCamera(Camera* camera);

		void AddToRenderQueue(std::shared_ptr<IDrawable> mesh);

	private:
		void Init(GLFWwindow& window);
		void CleanUp();

	public:
		RDeletionQueue deletionQueue;

	private:
		RVulkan* pRVulkan;
		EngineUI* pEngineUI;

		RenderQueue* renderQueue;

		Camera* camera;
	};
}