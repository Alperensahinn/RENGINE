#pragma once
#include <UI/Panel.h>

namespace REditor
{
	class SceneOutliner : public Panel
	{
	public:
		SceneOutliner() = default;
		~SceneOutliner() = default;

		void BeginRender() override;
		void Render() override;
		void EndRender() override;

	private:
		void RenderSceneOutlinerContent();

	};
}