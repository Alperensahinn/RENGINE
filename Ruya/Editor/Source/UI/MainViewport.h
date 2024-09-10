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

		void Render() override;

	private:
		void RenderViewportContent();

		VkDescriptorSet drawImageDescriptorSet;
	};
}