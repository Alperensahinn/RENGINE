#pragma once
#include <Graphics/Vulkan/RVulkan.h>

namespace Ruya
{
	class Material
	{
	public:
		RVkMaterialInstance material;
		RVkMetallicRoughness::MaterialResources materialResources;
	};
}