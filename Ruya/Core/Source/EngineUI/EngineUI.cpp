#include "EngineUI.h"
#include "../Graphics/Renderer/Renderer.h"
#include <Engine.h>
#include "../../Editor/Source/UI/Panel.h"

namespace Ruya
{
	EngineUI::EngineUI(GLFWwindow& window, Renderer* pRenderer)
	{
		Init(window, pRenderer);
		SetupUIStyle();
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

		for(auto& panel : Engine::GetInstance().GetEditorPanels())
		{
			panel->Render();
		}

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

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

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

	void EngineUI::SetupUIStyle()
	{
		ImGuiStyle* style = &ImGui::GetStyle();
		ImVec4* colors = style->Colors;

		colors[ImGuiCol_Text] = ImVec4(0.800f, 0.800f, 0.800f, 1.000f); // Lighter text for visibility
		colors[ImGuiCol_TextDisabled] = ImVec4(0.400f, 0.400f, 0.400f, 1.000f); // Darker disabled text
		colors[ImGuiCol_WindowBg] = ImVec4(0.050f, 0.050f, 0.050f, 1.000f); // Near black window background
		colors[ImGuiCol_ChildBg] = ImVec4(0.070f, 0.070f, 0.070f, 0.000f); // Near black child background
		colors[ImGuiCol_PopupBg] = ImVec4(0.080f, 0.080f, 0.080f, 1.000f); // Near black popup background
		colors[ImGuiCol_Border] = ImVec4(0.030f, 0.030f, 0.030f, 1.000f); // Almost black border
		colors[ImGuiCol_BorderShadow] = ImVec4(0.000f, 0.000f, 0.000f, 0.000f); // Shadow unchanged
		colors[ImGuiCol_FrameBg] = ImVec4(0.040f, 0.040f, 0.040f, 1.000f); // Near black frame background
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.060f, 0.060f, 0.060f, 1.000f); // Slightly lighter frame background hover
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.070f, 0.070f, 0.070f, 1.000f); // Slightly lighter frame background active
		colors[ImGuiCol_TitleBg] = ImVec4(0.030f, 0.030f, 0.030f, 1.000f); // Almost black title background
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.030f, 0.030f, 0.030f, 1.000f); // Almost black title background active
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.030f, 0.030f, 0.030f, 1.000f); // Almost black title background collapsed
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.050f, 0.050f, 0.050f, 1.000f); // Near black menu bar background
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.040f, 0.040f, 0.040f, 1.000f); // Near black scrollbar background
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.070f, 0.070f, 0.070f, 1.000f); // Slightly lighter scrollbar grab
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.080f, 0.080f, 0.080f, 1.000f); // Slightly lighter scrollbar grab hover
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.000f, 0.200f, 0.000f, 1.000f); // Brighter scrollbar grab active (unchanged)
		colors[ImGuiCol_CheckMark] = ImVec4(0.800f, 0.800f, 0.800f, 1.000f); // Lighter checkmark for visibility
		colors[ImGuiCol_SliderGrab] = ImVec4(0.060f, 0.060f, 0.060f, 1.000f); // Near black slider grab
		colors[ImGuiCol_SliderGrabActive] = ImVec4(1.000f, 0.200f, 0.000f, 1.000f); // Brighter slider grab active (unchanged)
		colors[ImGuiCol_Button] = ImVec4(0.800f, 0.800f, 0.800f, 0.000f); // Lighter button background
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.800f, 0.800f, 0.800f, 0.200f); // Lighter button background hover
		colors[ImGuiCol_ButtonActive] = ImVec4(0.800f, 0.800f, 0.800f, 0.400f); // Lighter button background active
		colors[ImGuiCol_Header] = ImVec4(0.070f, 0.070f, 0.070f, 1.000f); // Near black header
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.090f, 0.090f, 0.090f, 1.000f); // Slightly lighter header hover
		colors[ImGuiCol_HeaderActive] = ImVec4(0.090f, 0.090f, 0.090f, 1.000f); // Slightly lighter header active
		colors[ImGuiCol_Separator] = colors[ImGuiCol_Border]; // Separator unchanged
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.080f, 0.080f, 0.080f, 1.000f); // Slightly lighter separator hover
		colors[ImGuiCol_SeparatorActive] = ImVec4(1.000f, 0.200f, 0.000f, 1.000f); // Brighter separator active (unchanged)
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.800f, 0.800f, 0.800f, 0.250f); // Lighter resize grip
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.800f, 0.800f, 0.800f, 0.670f); // Lighter resize grip hover
		colors[ImGuiCol_ResizeGripActive] = ImVec4(1.000f, 1.000f, 1.000f, 0.700f); // Brighter resize grip active (unchanged)
		colors[ImGuiCol_Tab] = ImVec4(0.020f, 0.020f, 0.020f, 1.000f); // Almost black tab
		colors[ImGuiCol_TabHovered] = ImVec4(0.070f, 0.070f, 0.070f, 1.000f); // Slightly lighter tab hover
		colors[ImGuiCol_TabActive] = ImVec4(0.030f, 0.030f, 0.030f, 1.000f); // Almost black tab active
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.020f, 0.020f, 0.020f, 1.000f); // Almost black unfocused tab
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.030f, 0.030f, 0.030f, 1.000f); // Almost black unfocused active tab
		colors[ImGuiCol_DockingPreview] = ImVec4(1.000f, 0.200f, 0.000f, 0.781f); // Brighter docking preview (unchanged)
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.050f, 0.050f, 0.050f, 1.000f); // Near black docking empty background
		colors[ImGuiCol_PlotLines] = ImVec4(0.090f, 0.090f, 0.090f, 1.000f); // Near black plot lines
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.000f, 0.200f, 0.000f, 1.000f); // Brighter plot lines hovered (unchanged)
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.120f, 0.120f, 0.120f, 1.000f); // Near black plot histogram
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.000f, 0.200f, 0.000f, 1.000f); // Brighter plot histogram hovered (unchanged)
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.800f, 0.800f, 0.800f, 0.200f); // Lighter text selected background
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.000f, 0.200f, 0.000f, 1.000f); // Brighter drag and drop target (unchanged)
		colors[ImGuiCol_NavHighlight] = ImVec4(1.000f, 0.200f, 0.000f, 1.000f); // Brighter navigation highlight (unchanged)
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.000f, 0.200f, 0.000f, 1.000f); // Brighter navigation windowing highlight (unchanged)
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.700f); // Darker navigation windowing dim background
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.700f); // Darker modal window dim background

		style->ChildRounding = 4.0f;
		style->FrameBorderSize = 1.0f;
		style->FrameRounding = 2.0f;
		style->GrabMinSize = 7.0f;
		style->PopupRounding = 2.0f;
		style->ScrollbarRounding = 12.0f;
		style->ScrollbarSize = 13.0f;
		style->TabBorderSize = 1.0f;
		style->TabRounding = 0.0f;
		style->WindowRounding = 4.0f;
	}

	void EngineUI::CleanUp()
	{
		deletionQueue.flush();
	}

	unsigned int Color(unsigned int c)
	{
		const short a = 0xFF;
		const short r = (c >> 16) & 0xFF;
		const short g = (c >> 8) & 0xFF;
		const short b = (c >> 0) & 0xFF;
		return(a << 24)
			| (r << 0)
			| (g << 8)
			| (b << 16);
	}

	ImVec4 ColorConvertU32ToFloat4(uint32_t color)
	{
		float r = ((color >> 24) & 0xFF) / 255.0f;
		float g = ((color >> 16) & 0xFF) / 255.0f;
		float b = ((color >> 8) & 0xFF) / 255.0f;
		float a = ((color) & 0xFF) / 255.0f;
		return ImVec4(r, g, b, a);
	}
}
