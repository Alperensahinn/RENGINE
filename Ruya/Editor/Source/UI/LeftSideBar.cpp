#include "LeftSideBar.h"

void REditor::LeftSideBar::BeginRender()
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar;
    ImGui::Begin("##LeftSideMenuBar", nullptr, window_flags);
}

void REditor::LeftSideBar::Render()
{
    BeginRender();
    RenderLeftSideBarContent();
    EndRender();
}

void REditor::LeftSideBar::EndRender()
{
    Panel::EndRender();

    ImGui::End();
}

void REditor::LeftSideBar::RenderLeftSideBarContent()
{

}
