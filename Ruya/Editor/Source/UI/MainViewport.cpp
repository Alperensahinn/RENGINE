#include "MainViewport.h"
#include <EngineUI/EngineUI.h>

REditor::MainViewport::MainViewport() : Panel()
{
    drawImageDescriptorSet = ImGui_ImplVulkan_AddTexture(Ruya::Engine::GetInstance().GetRenderer().GetRendererBackend()->defaultSampler, Ruya::Engine::GetInstance().GetRenderer().GetRendererBackend()->drawImage.imageView, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

REditor::MainViewport::~MainViewport()
{
}

void REditor::MainViewport::Render()
{
    Panel::Render();

    ImGui::Begin("Main Viewport");
    RenderViewportContent();
    ImGui::End();
}

void REditor::MainViewport::RenderViewportContent()
{
    ImVec2 imageSize = ImVec2(Ruya::Engine::GetInstance().GetRenderer().GetRendererBackend()->drawImage.imageExtent.width,
        Ruya::Engine::GetInstance().GetRenderer().GetRendererBackend()->drawImage.imageExtent.height);
    ImGui::Image(drawImageDescriptorSet, imageSize);
}
