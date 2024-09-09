#pragma once
#include "GPUMesh.h"
#include "Material.h"
#include <Engine.h>

namespace Ruya
{
	struct RenderObject
	{
		glm::mat4 transform;
		PBRMaterial material;
		RVkMeshBuffer meshBuffer;

		void Destroy()
		{
			meshBuffer.Destroy(Engine::GetInstance().GetRenderer().pRVulkan);
			material.Destroy();
		}

	};
}