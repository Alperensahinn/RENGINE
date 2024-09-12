#pragma once
#include <UI/Panel.h>

namespace REditor
{
	class LeftSideBar : public Panel
	{
	public:
		LeftSideBar() = default;
		~LeftSideBar() = default;

		void BeginRender() override;
		void Render() override;
		void EndRender() override;

	private:
		void RenderLeftSideBarContent();

	};
}