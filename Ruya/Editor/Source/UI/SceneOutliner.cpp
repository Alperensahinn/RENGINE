#include "SceneOutliner.h"
#include <Engine.h>

void REditor::SceneOutliner::BeginRender()
{

}

void REditor::SceneOutliner::Render()
{
    BeginRender();
    RenderSceneOutlinerContent();
    EndRender();
}

void REditor::SceneOutliner::EndRender()
{
    Panel::EndRender();
    ImGui::End();
}

void REditor::SceneOutliner::RenderSceneOutlinerContent()
{
    std::shared_ptr<Ruya::RGame> game = Ruya::Engine::GetInstance().GetGame();

    std::vector<Ruya::Actor> sceneActors;

    for (const auto& scenePair : game->GetScenes())
    {
        for (const Ruya::Actor& actor : scenePair.second->GetActors())
        {
            sceneActors.push_back(actor);
        }
    }

    if (ImGui::Begin("Scene Outliner"))
    {
        static int selected = 0;
        {
            ImVec2 availableSize = ImGui::GetContentRegionAvail();
            ImGui::BeginChild("Scene Outliner Panel", ImVec2(availableSize.x, 0));
            for (int i = 0; i < sceneActors.size(); i++)
            {
                char label[128];
                sprintf(label, "%s", sceneActors[i].name.c_str());
                if (ImGui::Selectable(label, selected == i))
                    selected = i;
            }
        }

        ImGui::SameLine();
        ImGui::EndChild();
    }
}
