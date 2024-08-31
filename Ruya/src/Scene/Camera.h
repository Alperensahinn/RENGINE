#pragma once
#include "../Utilities/Math/RMath.h"

namespace Ruya
{
	class Camera
	{
		struct Transform
		{
			math::vec3 position;
			math::vec3 target = math::vec3(0.0f, 0.0f, 0.0f);
			math::vec3 direction = glm::normalize(position - target);
			math::vec3 worldUp = math::vec3(0.0f, 1.0f, 0.0f);
			math::vec3 up = glm::cross(direction, right);
			math::vec3 right = glm::normalize(glm::cross(worldUp, direction));
			math::vec3 front = math::vec3(0.0f, 0.0f, -1.0f);
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
		math::mat4 view;
	};
}