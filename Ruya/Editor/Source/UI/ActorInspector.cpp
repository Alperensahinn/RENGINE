#include "ActorInspector.h"

REditor::ActorInspector::ActorInspector()
{
}

REditor::ActorInspector::~ActorInspector()
{
}

void REditor::ActorInspector::BeginRender()
{
    ImGui::Begin("Actor Inspector");
}

void REditor::ActorInspector::Render()
{
	BeginRender();
	RenderActorInspectorContent();
	RenderTransformPanel();
	EndRender();
}

void REditor::ActorInspector::EndRender()
{
	Panel::EndRender();

	ImGui::End();
}

void REditor::ActorInspector::RenderActorInspectorContent()
{ 

}

void REditor::ActorInspector::RenderTransformPanel()
{
	ImVec2 availableSize = ImGui::GetContentRegionAvail();
	if (ImGui::BeginChild("Transform", ImVec2(availableSize.x, 0.0f)))
	{
		float position[3] = { 0.0f, 0.0f, 0.0f };
		float rotation[3] = { 0.0f, 0.0f, 0.0f };
		float scale[3] = { 1.0f, 1.0f, 1.0f };

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Position");
		ImGui::SameLine();
		if (ImGui::InputFloat3("##Position", position))
		{

		}

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Rotation");
		ImGui::SameLine();
		if (ImGui::InputFloat3("##Rotation", rotation))
		{

		}

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Scale   ");
		ImGui::SameLine();
		if (ImGui::InputFloat3("##Scale", scale))
		{

		}

		ImGui::EndChild();
	}
}
