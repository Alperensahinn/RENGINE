#include "Material.h"
#include <Engine.h>

void Ruya::PBRMaterial::Init(MaterialResources resources)
{
	this->resources = resources;

	descriptorSetMaterial = Engine::GetInstance().GetRenderer().pRVulkan->descriptorAllocator.Allocate(Engine::GetInstance().GetRenderer().pRVulkan, Engine::GetInstance().GetRenderer().pRVulkan->pbrPipelineDescriptorSetLayoutMaterial, nullptr);

	writer.Clear();

	writer.Clear();
	writer.WriteImage(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, resources.albedoTexture.image.imageView, resources.albedoTexture.sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	writer.WriteImage(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, resources.normalTexture.image.imageView, resources.normalTexture.sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	writer.UpdateDescriptorSets(Engine::GetInstance().GetRenderer().pRVulkan, descriptorSetMaterial);
}

void Ruya::PBRMaterial::SetResources(MaterialResources resources)
{
	this->resources = resources;

	writer.Clear();

	writer.WriteImage(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, resources.albedoTexture.image.imageView, resources.albedoTexture.sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	writer.WriteImage(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, resources.normalTexture.image.imageView, resources.normalTexture.sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	writer.UpdateDescriptorSets(Engine::GetInstance().GetRenderer().pRVulkan, descriptorSetMaterial);
}

void Ruya::PBRMaterial::Destroy()
{
	resources.albedoTexture.Destroy();
	resources.normalTexture.Destroy();
}


