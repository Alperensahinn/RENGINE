#pragma once
#include "../Utilities/Math/RMath.h"
#include <string>

namespace Ruya
{
	struct Vertex
	{
		glm::vec3 position;
		float uv_x;
		glm::vec3 normal;
		float uv_y;
		glm::vec4 color;
	};

	struct GeometrySurface
	{
		uint32_t startIndex;
		uint32_t count;
	};

	struct Mesh
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
	};

	void BuildModel();
}