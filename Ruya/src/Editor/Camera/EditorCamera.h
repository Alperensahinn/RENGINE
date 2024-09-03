#pragma once
#include "../../Engine/Scene/Camera.h"

namespace REditor
{
	class EditorCamera : public Ruya::Camera
	{
	public:
		EditorCamera();
		~EditorCamera();

		EditorCamera(const EditorCamera&) = delete;
		EditorCamera& operator=(const EditorCamera&) = delete;

	public:
		void Update();

	private:
		Ruya::math::vec3 direction;
		float pitch;
		float yaw;
	};
}