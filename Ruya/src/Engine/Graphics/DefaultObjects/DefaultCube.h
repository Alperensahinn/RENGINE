#pragma once
#include "../../Scene/Model.h"
#include <array>

namespace Ruya
{
	struct DefaultCube
	{
		std::vector<Vertex> GetVertices()
		{
			std::array<Vertex, 4> rect_vertices;

			rect_vertices[0].position = { 0.5,-0.5, 0 };
			rect_vertices[1].position = { 0.5,0.5, 0 };
			rect_vertices[2].position = { -0.5,-0.5, 0 };
			rect_vertices[3].position = { -0.5,0.5, 0 };

			std::vector<Vertex> vertices;

			for(int i = 0; i < 4; i++)
			{
				vertices.push_back(rect_vertices[i]);
			}

			return vertices;
		}

		std::vector<uint32_t> GetIndices()
		{

			std::array<uint32_t, 6> rect_indices;

			rect_indices[0] = 0;
			rect_indices[1] = 1;
			rect_indices[2] = 2;

			rect_indices[3] = 2;
			rect_indices[4] = 1;
			rect_indices[5] = 3;

			std::vector<uint32_t> indices;

			for (int i = 0; i < 6; i++)
			{
				indices.push_back(rect_indices[i]);
			}

			return indices;
		}
	};
}