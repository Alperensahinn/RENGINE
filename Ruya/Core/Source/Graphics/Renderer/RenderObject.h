#pragma once
#include "GPUMesh.h"
#include "Material.h"

namespace Ruya
{
	struct RenderObject
	{
		glm::mat4 transform;
		PBRMaterial material;
		RVkMeshBuffer meshBuffer;
	};
}