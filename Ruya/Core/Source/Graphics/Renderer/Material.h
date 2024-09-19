#pragma once
#include <Graphics/Vulkan/RVulkan.h>
#include "Texture.h"

namespace Ruya
{

	struct PBRMaterial
	{
		struct MaterialResources
		{
			Texture albedoTexture;
			Texture normalTexture;
			Texture roughnessMetalicTexture;
		}resources;

		VkDescriptorSet descriptorSetMaterial;

		RVkDescriptorWriter writer;

		void Init(MaterialResources resources);
		void SetResources(MaterialResources resources);
		void Destroy();
	};
}