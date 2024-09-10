#pragma once

namespace REditor
{
	class Panel
	{
	public:
		Panel() = default;
		~Panel() = default;

		virtual void Render();
	};
}