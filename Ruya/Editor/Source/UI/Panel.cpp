#include "Panel.h"

void REditor::Panel::RenderChildPanels()
{
	for(auto& panel : childPanels)
	{
		panel->Render();
	}
}

void REditor::Panel::EndRender()
{
	RenderChildPanels();
}
