#include "ProjectB.h"
#include <GameFramework/Scene.h>
#include <GameFramework/Entity.h>
#include <GameFramework/MeshComponent.h>
#include <GameFramework/TransformComponent.h>
#include <GameFramework/MeshRenderSystem.h>
#include <Utilities/FileSystem/FileSystem.h>
#include <Graphics/Renderer/RenderObject.h>
#include <array>
#include <Graphics/Renderer/Texture.h>
#include <Engine.h>
#include <Graphics/Renderer/Material.h>
#include <stb_image.h>
#include <filesystem>

ProjectB::ProjectB()
{
	std::shared_ptr<Ruya::Scene> mainScene = AddScene(std::make_shared<Ruya::Scene>());
	mainScene->AddSceneSystem<Ruya::MeshRenderSystem>();

	Ruya::EntityID monkey = mainScene->NewEntity();
	mainScene->GetEntity(monkey)->name = "MandolorianHelmet";

	Ruya::TransformComponent& monkeyTransformComponent = mainScene->AddComponent<Ruya::TransformComponent>(monkey);

	Ruya::MeshComponent& monkeyMeshComponent = mainScene->AddComponent<Ruya::MeshComponent>(monkey);

	std::filesystem::path scriptPath = __FILE__;

	std::filesystem::path solutionPath = scriptPath.parent_path().parent_path();

	std::shared_ptr<Ruya::Mesh> mesh = Ruya::ImportFBXMesh(solutionPath.string() + "\\TestMeshes\\MandolorianHelmet\\MandolorianHelmet.fbx");

	Ruya::Texture albedoTexture = Ruya::LoadTexture(solutionPath.string() + "\\TestMeshes\\MandolorianHelmet\\Mando_Helm_Mat_baseColor.png", VK_FORMAT_R8G8B8A8_SRGB);

	Ruya::Texture normalTexture = Ruya::LoadTexture(solutionPath.string() + "\\TestMeshes\\MandolorianHelmet\\Mando_Helm_Mat_normal.png", VK_FORMAT_R8G8B8A8_UNORM);

	Ruya::Texture roughnessMetalicTexture = Ruya::LoadTexture(solutionPath.string() + "\\TestMeshes\\MandolorianHelmet\\Mando_Helm_Mat_metallicRoughness.png", VK_FORMAT_R8G8B8A8_UNORM);

	std::shared_ptr<Ruya::RenderObject> renderObject = std::make_shared<Ruya::RenderObject>(Ruya::Engine::GetInstance().GetRenderer().CreateRenderObject(mesh, albedoTexture, normalTexture, roughnessMetalicTexture));
	monkeyMeshComponent.renderObject = renderObject;
}

ProjectB::~ProjectB()
{
}
