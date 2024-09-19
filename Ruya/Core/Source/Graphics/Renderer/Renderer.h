#pragma once
#include "../Vulkan/RVulkan.h"
#include "../../EngineUI/EngineUI.h"
#include "../../Collections/RDeletionQueue.h"
#include "../Mesh.h"

struct GLFWwindow;

namespace Ruya
{
	class RVulkan;
	class Camera;
	class RenderQueue;
	struct Texture;
	struct RenderObject;

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

		RenderObject CreateRenderObject(std::shared_ptr<Mesh> mesh, Texture albedoTexture, Texture normalTexture, Texture roughnessMetalicTexture);
		void BindCamera(Camera* camera);

		void AddToRenderQueue(std::shared_ptr<RenderObject> renderObject);

	private:
		void Init(GLFWwindow& window);
		void CleanUp();

	public:
		RDeletionQueue deletionQueue;

	public:
		RVulkan* pRVulkan;
		EngineUI* pEngineUI;

		RenderQueue* renderQueue;

		Camera* camera;
	};
}