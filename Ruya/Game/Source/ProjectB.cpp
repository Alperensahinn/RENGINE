#include "ProjectB.h"
#include <GameFramework/Scene.h>
#include <GameFramework/Actor.h>
#include <GameFramework/MeshComponent.h>
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

	Ruya::ActorID monkey = mainScene->NewActor();
	mainScene->GetActor(monkey)->name = "MandolorianHelmet";

	Ruya::MeshComponent* monkeyMeshComponent = mainScene->AddComponent<Ruya::MeshComponent>(monkey);

	std::filesystem::path scriptPath = __FILE__;

	std::filesystem::path solutionPath = scriptPath.parent_path().parent_path();

	std::shared_ptr<Ruya::Mesh> mesh = Ruya::ImportFBXMesh(solutionPath.string() + "\\TestMeshes\\MandolorianHelmet\\MandolorianHelmet.fbx");

	Ruya::Texture albedoTexture = Ruya::LoadTexture(solutionPath.string() + "\\TestMeshes\\MandolorianHelmet\\Mando_Helm_Mat_baseColor.png", VK_FORMAT_R8G8B8A8_SRGB);

	Ruya::Texture normalTexture = Ruya::LoadTexture(solutionPath.string() + "\\TestMeshes\\MandolorianHelmet\\Mando_Helm_Mat_normal.png", VK_FORMAT_R8G8B8A8_UNORM);

	Ruya::Texture roughnessMetalicTexture = Ruya::LoadTexture(solutionPath.string() + "\\TestMeshes\\MandolorianHelmet\\Mando_Helm_Mat_metallicRoughness.png", VK_FORMAT_R8G8B8A8_UNORM);

	std::shared_ptr<Ruya::RenderObject> renderObject = std::make_shared<Ruya::RenderObject>(Ruya::Engine::GetInstance().GetRenderer().CreateRenderObject(mesh, albedoTexture, normalTexture, roughnessMetalicTexture));
	monkeyMeshComponent->SetRenderObject(renderObject);
}

ProjectB::~ProjectB()
{
}
