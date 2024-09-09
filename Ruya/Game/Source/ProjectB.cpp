#include "ProjectB.h"
#include <Scene/Scene.h>
#include <GameFramework/Actor.h>
#include <GameFramework/MeshComponent.h>
#include <Utilities/FileSystem/FileSystem.h>
#include <Graphics/Renderer/RenderObject.h>
#include <array>
#include <Graphics/Renderer/Texture.h>
#include <Engine.h>
#include <Graphics/Renderer/Material.h>

ProjectB::ProjectB()
{
	std::unique_ptr<Ruya::Scene>& mainScene = AddScene(std::make_unique<Ruya::Scene>());

	std::unique_ptr<Ruya::Actor>& monkey = mainScene->AddActor(std::make_unique<Ruya::Actor>());

	std::unique_ptr<Ruya::ActorComponent>& monkeyMesh = monkey->AddComponent(std::make_unique<Ruya::MeshComponent>());

	Ruya::MeshComponent* meshComponent = dynamic_cast<Ruya::MeshComponent*>(monkeyMesh.get());

	uint32_t magenta = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));
	uint32_t black = glm::packUnorm4x8(glm::vec4(0, 0, 0, 0));
	std::array<uint32_t, 16 * 16 > pixels;

	for (int x = 0; x < 16; x++)
	{
		for (int y = 0; y < 16; y++)
		{
			pixels[y * 16 + x] = ((x % 2) ^ (y % 2)) ? magenta : black;
		}
	}

	std::shared_ptr<Ruya::Mesh> mesh = Ruya::ImportFBXMesh("C:\\Users\\aalpe\\Desktop\\RENGINE\\Ruya\\Game\\Source\\TestMeshes\\Monkey.glb");
	Ruya::Texture texture;
	texture.image = Ruya::rvkCreateImage(Ruya::Engine::GetInstance().GetRenderer().pRVulkan, pixels.data(), VkExtent3D{ 16, 16, 1 }, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	
	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
	samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
	
	VkSampler sampler;
	vkCreateSampler(Ruya::Engine::GetInstance().GetRenderer().pRVulkan->pDevice, &samplerCreateInfo, nullptr, &sampler);
	texture.sampler = sampler;

	std::shared_ptr<Ruya::RenderObject> renderObject = std::make_shared<Ruya::RenderObject>(Ruya::Engine::GetInstance().GetRenderer().CreateRenderObject(mesh, texture));
	meshComponent->SetRenderObject(renderObject);
}

ProjectB::~ProjectB()
{
}
