#pragma once
#include <Graphics/Vulkan/RVulkan.h>

namespace Ruya
{
	struct Texture
	{
		RVkAllocatedImage image;
		VkSampler sampler;
	};
}