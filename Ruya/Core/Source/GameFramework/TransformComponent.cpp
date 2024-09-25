#include "TransformComponent.h"

Ruya::math::mat4 Ruya::TransformComponent::GetTransformMatrix()
{
    math::mat4 worldMatrix;
    worldMatrix = math::mat4(1.0f);
    worldMatrix = glm::translate(worldMatrix, position);
    worldMatrix = glm::scale(worldMatrix, scale);
    return worldMatrix;
}
