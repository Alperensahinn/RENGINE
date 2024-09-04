#pragma once

#define GLM_FORCE_INTRINSICS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Ruya
{
	namespace math
	{
		typedef glm::vec1 vec1;
		typedef glm::vec2 vec2;
		typedef glm::vec3 vec3;
		typedef glm::vec4 vec4;

		typedef glm::mat3 mat3;
		typedef glm::mat4 mat4;

		float Dot(vec1 v1, vec1 v2);
		float Dot(vec2 v1, vec2 v2);
		float Dot(vec3 v1, vec3 v2);
		float Dot(vec4 v1, vec4 v2);

		vec1 Normalize(vec1 v);
		vec2 Normalize(vec2 v);
		vec3 Normalize(vec3 v);
		vec4 Normalize(vec4 v);

		vec3 Cross(vec3 v1, vec3 v2);
	}
}