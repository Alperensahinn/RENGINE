#pragma once
#include <UI/Panel.h>

namespace REditor
{
	class ActorInspector : public Panel
	{
	public:
		ActorInspector();
		~ActorInspector();

		void BeginRender() override;
		void Render() override;
		void EndRender() override;

	private:
		void RenderActorInspectorContent();
		void RenderTransformPanel();

	};
}