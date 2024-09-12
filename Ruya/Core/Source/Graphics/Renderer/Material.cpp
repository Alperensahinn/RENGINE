#include "Material.h"
#include <Engine.h>


void Ruya::PBRMaterial::Init(MaterialResources resources)
{
	this->resources = resources;

	descriptorSetUniform = Engine::GetInstance().GetRenderer().pRVulkan->descriptorAllocator.Allocate(Engine::GetInstance().GetRenderer().pRVulkan, Engine::GetInstance().GetRenderer().pRVulkan->pbrPipelineDescriptorSetLayoutUniform, nullptr);
	descriptorSetMaterial = Engine::GetInstance().GetRenderer().pRVulkan->descriptorAllocator.Allocate(Engine::GetInstance().GetRenderer().pRVulkan, Engine::GetInstance().GetRenderer().pRVulkan->pbrPipelineDescriptorSetLayoutMaterial, nullptr);

	writer.Clear();

	struct UniformBuffer
	{
		math::mat4 view;
		math::mat4 proj;
		math::mat4 viewproj;
	};

	Ruya::RVkAllocatedBuffer buffer = Ruya::rvkCreateBuffer(Engine::GetInstance().GetRenderer().pRVulkan, sizeof(UniformBuffer), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	UniformBuffer bufferData;
	bufferData.view = math::mat4(1.0f);
	bufferData.proj = math::mat4(1.0f);
	bufferData.viewproj = math::mat4(1.0f);

	void* data;
	vmaMapMemory(Engine::GetInstance().GetRenderer().pRVulkan->vmaAllocator, buffer.vmaAllocation, &data);
	memcpy(data, &bufferData, sizeof(UniformBuffer));
	vmaUnmapMemory(Engine::GetInstance().GetRenderer().pRVulkan->vmaAllocator, buffer.vmaAllocation);

	writer.WriteBuffer(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, buffer.vkBuffer, sizeof(UniformBuffer), 0);
	writer.UpdateDescriptorSets(Engine::GetInstance().GetRenderer().pRVulkan, descriptorSetUniform);

	writer.Clear();
	writer.WriteImage(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, resources.albedoTexture.image.imageView, resources.albedoTexture.sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	writer.UpdateDescriptorSets(Engine::GetInstance().GetRenderer().pRVulkan, descriptorSetMaterial);
}

void Ruya::PBRMaterial::SetResources(MaterialResources resources)
{
	this->resources = resources;

	writer.Clear();

	writer.WriteImage(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, resources.albedoTexture.image.imageView, resources.albedoTexture.sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	writer.UpdateDescriptorSets(Engine::GetInstance().GetRenderer().pRVulkan, descriptorSetMaterial);
}

void Ruya::PBRMaterial::Destroy()
{
	resources.albedoTexture.Destroy();
}


