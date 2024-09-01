#pragma once
#include "../../Engine/Engine.h"

namespace Ruya::Editor
{
	class App
	{
	public:
		void Run();

	private:
		void Init();
		void CleanUp();

	private:
		Engine* engine;
	};
}