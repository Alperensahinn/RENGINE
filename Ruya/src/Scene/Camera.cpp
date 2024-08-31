#include "Camera.h"

Ruya::Camera::Camera() : view(math::mat4(1.0f))
{
	transform.position = math::vec3(0.0f, 0.0f, -5.0f);
}

Ruya::Camera::~Camera()
{
}

Ruya::math::mat4 Ruya::Camera::GetViewMatrix()
{
	glm::mat4 viewMatrix = glm::mat4(1.0f);
	viewMatrix = glm::translate(view, transform.position);

	return viewMatrix;
}

