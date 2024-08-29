#pragma once
#include "../../Scene/Mesh.h"
#include <array>

namespace Ruya
{
	struct DefaultCube
	{
		std::array<Vertex, 4> GetVertices()
		{
			std::array<Vertex, 4> vertices;

			vertices[0].position = { 0.5,-0.5, 0 };
			vertices[1].position = { 0.5,0.5, 0 };
			vertices[2].position = { -0.5,-0.5, 0 };
			vertices[3].position = { -0.5,0.5, 0 };
			vertices[0].color = { 0,0, 0,1 };
			vertices[1].color = { 0.5,0.5,0.5 ,1 };
			vertices[2].color = { 1,0, 0,1 };
			vertices[3].color = { 0,1, 0,1 };

			return vertices;
		}

		std::array<uint32_t, 6> GetIndices()
		{
			std::array<uint32_t, 6> indices;

			indices[0] = 0;
			indices[1] = 1;
			indices[2] = 2;
			indices[3] = 2;
			indices[4] = 1;
			indices[5] = 3;


			return indices;
		}
	};
}