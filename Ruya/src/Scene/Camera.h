#pragma once
#include "../Utilities/Math/RMath.h"

namespace Ruya
{
	class Camera
	{
		struct Transform
		{
			glm::vec3 position = math::vec3(0.0f, 0.0f, 5.0f);
			glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
			glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

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