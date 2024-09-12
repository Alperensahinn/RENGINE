#pragma once
#include <UI/Panel.h>

namespace REditor
{
	class MenuBar : public Panel
	{
	public:
		MenuBar();
		~MenuBar();

		void BeginRender() override;
		void Render() override;
		void EndRender() override;

	private:
		void RenderMenuBarContent();

	};
}