#include "Camera.h"

Ruya::Camera::Camera()
{
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
	right = glm::normalize(glm::cross(worldUp, direction));
	return right;
}
