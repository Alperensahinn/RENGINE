#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Ruya
{
	class Time
	{
	private:
		Time();
		virtual ~Time() = 0;

		Time(const Time&) = delete;
		Time& operator=(const Time&) = delete;

	public:
		static void UpdateTime();

	public:
		static float GetDeltaTime();

	private:
		static float deltaTime;
		static float lastFrameTime;
	};
}