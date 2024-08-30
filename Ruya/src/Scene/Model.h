#pragma once
#include "../Utilities/Math/RMath.h"
#include <string>

namespace Ruya
{
	struct Vertex
	{
		math::vec3 position;
		math::vec3 normal;
		math::vec2 uv;
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