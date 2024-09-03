#pragma once
#include "../Utilities/Math/RMath.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


namespace Ruya
{
	class Engine;

	class RInput
	{
	private:
		RInput();
		~RInput();

		RInput(const RInput&) = delete;
		RInput& operator=(const RInput&) = delete;

	public:
		enum class KeyCode
		{
			A,
			D,
			E,
			Q,
			S,
			W
		};

		enum class MouseButton
		{
			RIGHT,
			LEFT
		};
		static void Init();

		static bool GetKey(KeyCode keyCode);

		static bool GetMouseButton(MouseButton mouseButton);
		static math::vec2 GetMouseDelta();
		static void SetCursorEnabled(bool isEnable);

	public:
		static float firstMouse;
		static float mouseLastX;
		static float mouseLastY;
		static float mouse_offset_x;
		static float mouse_offset_y;
		static float mouse_offset_x_last;
		static float mouse_offset_y_last;
	};
}