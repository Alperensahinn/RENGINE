#include "EngineUI.h"
#include "../Graphics/Renderer/Renderer.h"

namespace Ruya
{
	EngineUI::EngineUI(GLFWwindow& window, Renderer* pRenderer)
	{
		Init(window, pRenderer);
	}

	EngineUI::~EngineUI()
	{
		CleanUp();
	}

	void EngineUI::Draw()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImVec2 position = ImVec2(500, 500); 
		ImGui::SetNextWindowPos(position, ImGuiCond_Once);

		ImGui::ShowDemoWindow();


		ImGui::Render();
	}
	
	void EngineUI::DrawData(VkCommandBuffer cmd)
	{
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
	}

	void EngineUI::Init(GLFWwindow& window, Renderer* pRenderer)
	{
		ImGui::CreateContext();

		ImGui_ImplGlfw_InitForVulkan(&window, true);

		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.Instance = pRenderer->GetRendererBackend()->pInstance;
		initInfo.PhysicalDevice = pRenderer->GetRendererBackend()->pPhysicalDevice;
		initInfo.Device = pRenderer->GetRendererBackend()->pDevice;
		initInfo.Queue = pRenderer->GetRendererBackend()->pGraphicsQueue;
		initInfo.DescriptorPool = pRenderer->GetRendererBackend()->immediateUIPool;
		initInfo.MinImageCount = 3;
		initInfo.ImageCount = 3;
		initInfo.UseDynamicRendering = true;
		initInfo.PipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
		initInfo.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
		initInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = &(pRenderer->GetRendererBackend()->swapChainImageFormat);
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

		ImGui_ImplVulkan_Init(&initInfo);

		ImGui_ImplVulkan_CreateFontsTexture();

		deletionQueue.PushFunction([=]()
			{
				ImGui_ImplVulkan_Shutdown();
			});
	}

	void EngineUI::CleanUp()
	{
		deletionQueue.flush();
	}
}
