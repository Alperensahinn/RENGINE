#pragma once
#include <UI/Panel.h>
#include <Engine.h>

namespace REditor
{
	class MainViewport : public Panel
	{
	public:
		MainViewport();
		~MainViewport();

		void BeginRender() override;
		void Render() override;
		void EndRender() override;

	private:
		void RenderViewportContent();


		VkDescriptorSet drawImageDescriptorSet;
	};
}