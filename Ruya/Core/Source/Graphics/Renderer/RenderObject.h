#pragma once
#include "Material.h"
#include <Engine.h>

namespace Ruya
{
	struct RenderObject
	{
		glm::mat4 modelMatrix;
		PBRMaterial material;
		RVkMeshBuffer meshBuffer;

		void Destroy()
		{
			meshBuffer.Destroy(Engine::GetInstance().GetRenderer().pRVulkan);
			material.Destroy();
		}

	};
}