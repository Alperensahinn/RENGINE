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
#include <stb_image.h>

ProjectB::ProjectB()
{
	std::shared_ptr<Ruya::Scene> mainScene = AddScene(std::make_shared<Ruya::Scene>());

	std::shared_ptr<Ruya::Actor> monkey = mainScene->AddActor(std::make_shared<Ruya::Actor>());
	monkey->name = "Monkey";

	std::unique_ptr<Ruya::ActorComponent>& monkeyMesh = monkey->AddComponent(std::make_unique<Ruya::MeshComponent>());

	Ruya::MeshComponent* meshComponent = dynamic_cast<Ruya::MeshComponent*>(monkeyMesh.get());

	std::shared_ptr<Ruya::Mesh> mesh = Ruya::ImportFBXMesh("C:\\Users\\aalpe\\Desktop\\RENGINE\\Ruya\\Game\\Source\\TestMeshes\\MandolorianHelmet\\MandolorianHelmet.fbx");
	
	int width, height, channels;
	stbi_uc* pixels = stbi_load("C:\\Users\\aalpe\\Desktop\\RENGINE\\Ruya\\Game\\Source\\TestMeshes\\MandolorianHelmet\\Mando_Helm_Mat_baseColor.png", &width, &height, &channels, STBI_rgb_alpha);

	if (!pixels) 
	{
		std::cerr << "Failed to load texture image!" << std::endl;
		return;
	}

	VkExtent3D imageExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };

	Ruya::Texture texture;
	texture.image = Ruya::rvkCreateImage(Ruya::Engine::GetInstance().GetRenderer().pRVulkan, pixels, imageExtent, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

	stbi_image_free(pixels);

	texture.sampler = Ruya::rvkCreateSamplerLinear(Ruya::Engine::GetInstance().GetRenderer().pRVulkan);

	std::shared_ptr<Ruya::RenderObject> renderObject = std::make_shared<Ruya::RenderObject>(Ruya::Engine::GetInstance().GetRenderer().CreateRenderObject(mesh, texture));
	meshComponent->SetRenderObject(renderObject);
}

ProjectB::~ProjectB()
{
}
