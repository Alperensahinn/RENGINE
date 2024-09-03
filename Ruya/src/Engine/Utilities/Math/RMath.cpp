#include "RMath.h"

float Ruya::math::Dot(vec1 v1, vec1 v2)
{
    return glm::dot(v1, v2);
}

float Ruya::math::Dot(vec2 v1, vec2 v2)
{
    return glm::dot(v1, v2);
}

float Ruya::math::Dot(vec3 v1, vec3 v2)
{
    return glm::dot(v1, v2);
}

float Ruya::math::Dot(vec4 v1, vec4 v2)
{
    return glm::dot(v1, v2);
}

Ruya::math::vec1 Ruya::math::Normalize(vec1 v)
{
    return glm::normalize(v);
}

Ruya::math::vec2 Ruya::math::Normalize(vec2 v)
{
    return glm::normalize(v);
}

Ruya::math::vec3 Ruya::math::Normalize(vec3 v)
{
    return glm::normalize(v);
}

Ruya::math::vec4 Ruya::math::Normalize(vec4 v)
{
    return glm::normalize(v);
}

Ruya::math::vec3 Ruya::math::Cross(vec3 v1, vec3 v2)
{
    return glm::cross(v1, v2);
}




