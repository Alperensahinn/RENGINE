#pragma once

#include "../Vendor/imgui/imgui.h"
#include "../Vendor/imgui/imgui_impl_glfw.h"
#include "../Vendor/imgui/imgui_impl_vulkan.h"


#include "../Collections/RDeletionQueue.h"

namespace Ruya
{
	class Renderer;

	class EngineUI
	{
	public:
		EngineUI(GLFWwindow& window, Renderer* pRenderer);
		~EngineUI();

		EngineUI(const EngineUI&) = delete;
		EngineUI& operator=(const EngineUI&) = delete;

	public:
		void Draw();
		void DrawData(VkCommandBuffer cmd);

	private:
		void Init(GLFWwindow& window, Renderer* pRenderer);
		void SetupUIStyle();
		void CleanUp();

	private:
		RDeletionQueue deletionQueue;
	};
}