#include "Camera.h"

Ruya::Camera::Camera()
{
	transform.position = math::vec3(0.0f, 0.0f, 5.0f);
	transform.front = glm::vec3(0.0f, 0.0f, -1.0f);
	transform.up = glm::vec3(0.0f, 1.0f, 0.0f);
}

Ruya::Camera::~Camera()
{
}

Ruya::math::mat4 Ruya::Camera::GetViewMatrix()
{
	return glm::lookAt(transform.position, transform.position + transform.front, transform.up);
}

Ruya::math::vec3 Ruya::Camera::Transform::GetRight()
{
	return glm::cross(front, up);
}
