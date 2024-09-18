#pragma once
#include "../Utilities/Math/RMath.h"

namespace Ruya
{
	struct Vertex
	{
		glm::vec3 position;
		float uv_x;
		glm::vec3 normal;
		float uv_y;
		glm::vec4 color;
		glm::vec3 tangent;
		float pad0;
		glm::vec3 biTangent;
		float pad1;
	};

	struct Mesh
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
	};
}