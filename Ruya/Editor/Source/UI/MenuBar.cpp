#include "MenuBar.h"

REditor::MenuBar::MenuBar()
{
}

REditor::MenuBar::~MenuBar()
{
}

void REditor::MenuBar::BeginRender()
{
}

void REditor::MenuBar::Render()
{
    BeginRender();
    RenderMenuBarContent();
    EndRender();
}

void REditor::MenuBar::EndRender()
{
	Panel::EndRender();
}

void REditor::MenuBar::RenderMenuBarContent()
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Engine"))
        {
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tools"))
        {
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug"))
        {
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help"))
        {
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}
