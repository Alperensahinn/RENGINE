#include "MainViewport.h"
#include <EngineUI/EngineUI.h>

REditor::MainViewport::MainViewport() : Panel()
{
    drawImageDescriptorSet = ImGui_ImplVulkan_AddTexture(Ruya::Engine::GetInstance().GetRenderer().GetRendererBackend()->defaultNearestSampler, Ruya::Engine::GetInstance().GetRenderer().GetRendererBackend()->drawImage.imageView, VK_IMAGE_LAYOUT_GENERAL);
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
    ImVec2 windowSize = ImGui::GetContentRegionAvail();

    ImVec2 imageSize = ImVec2(Ruya::Engine::GetInstance().GetRenderer().GetRendererBackend()->drawImage.imageExtent.width,
        Ruya::Engine::GetInstance().GetRenderer().GetRendererBackend()->drawImage.imageExtent.height);

    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(0, 0), ImVec2(imageSize.x, imageSize.y), IM_COL32(0, 0, 0, 255));

    ImVec2 cursorPos = ImVec2((ImGui::GetWindowSize().x - imageSize.x), (ImGui::GetWindowSize().y - imageSize.y));
    cursorPos.x = cursorPos.x * 0.5f;
    cursorPos.y = cursorPos.y * 0.5f;

    ImGui::SetCursorPos(cursorPos);
    ImGui::Image(drawImageDescriptorSet, imageSize);
}
