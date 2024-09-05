#pragma once
#include "GPUMesh.h"
#include "Material.h"

namespace Ruya
{
	class RenderObject
	{
	public:
		GPUMesh mesh;
		Material material;
	};
}