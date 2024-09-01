#pragma once
#include "../Utilities/Math/RMath.h"

namespace Ruya
{
	class Camera
	{
		struct Transform
		{
			glm::vec3 position;
			glm::vec3 front;
			glm::vec3 up;

			math::vec3 GetRight();
		};

	public:
		Camera();
		~Camera();

		Camera(const Camera&) = delete;
		Camera& operator=(const Camera&) = delete;

	public:
		math::mat4 GetViewMatrix();

	public:
		Transform transform;

	};
}