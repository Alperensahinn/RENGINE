#pragma once
#include <vector>

namespace REditor
{
	class Panel
	{
	public:
		Panel() = default;
		~Panel() = default;

		virtual void BeginRender() = 0;
		virtual void Render() = 0;
		virtual void EndRender();

	private:
		void RenderChildPanels();

	public:
		std::vector<Panel*> childPanels;
	};
}