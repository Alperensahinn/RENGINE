#pragma once
#include <Graphics/Vulkan/RVulkan.h>
#include <Engine.h>

namespace Ruya
{
	struct Texture
	{
		RVkAllocatedImage image;
		VkSampler sampler;

		void Destroy()
		{
			Ruya::rvkDestroyImage(Engine::GetInstance().GetRenderer().pRVulkan, image);
		};
	};
}