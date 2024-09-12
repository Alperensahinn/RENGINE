#include "MainViewport.h"
#include <EngineUI/EngineUI.h>

REditor::MainViewport::MainViewport() : Panel()
{
    drawImageDescriptorSet = ImGui_ImplVulkan_AddTexture(Ruya::Engine::GetInstance().GetRenderer().GetRendererBackend()->defaultSampler, Ruya::Engine::GetInstance().GetRenderer().GetRendererBackend()->drawImage.imageView, VK_IMAGE_LAYOUT_GENERAL);
}

REditor::MainViewport::~MainViewport()
{
}

void REditor::MainViewport::Render()
{
    BeginRender();
    RenderViewportContent();
    EndRender();
}

void REditor::MainViewport::BeginRender()
{
    ImGui::Begin("Main Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
}

void REditor::MainViewport::EndRender()
{
    Panel::EndRender();

    ImGui::End();
}

void REditor::MainViewport::RenderViewportContent()
{
    ImVec2 imageSize = ImVec2(Ruya::Engine::GetInstance().GetRenderer().GetRendererBackend()->drawImage.imageExtent.width,
        Ruya::Engine::GetInstance().GetRenderer().GetRendererBackend()->drawImage.imageExtent.height);

    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(0, 0), ImVec2(imageSize.x, imageSize.y), IM_COL32(0, 0, 0, 0));

    ImVec2 windowSize = ImGui::GetWindowSize();
    ImVec2 position = ImVec2(
        (windowSize.x - imageSize.x) * 0.5f,
        (windowSize.y - imageSize.y) * 0.5f
    );

    ImGui::SetCursorPos(position);

    ImGui::Image(drawImageDescriptorSet, imageSize);
}
