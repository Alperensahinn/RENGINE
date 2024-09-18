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
	monkey->name = "MandolorianHelmet";

	std::unique_ptr<Ruya::ActorComponent>& monkeyMesh = monkey->AddComponent(std::make_unique<Ruya::MeshComponent>());

	Ruya::MeshComponent* meshComponent = dynamic_cast<Ruya::MeshComponent*>(monkeyMesh.get());

	std::shared_ptr<Ruya::Mesh> mesh = Ruya::ImportFBXMesh("C:\\Users\\aalpe\\Desktop\\RENGINE\\Ruya\\Game\\Source\\TestMeshes\\MandolorianHelmet\\MandolorianHelmet.fbx");
	
	Ruya::Texture albedoTexture = Ruya::LoadTexture("C:\\Users\\aalpe\\Desktop\\RENGINE\\Ruya\\Game\\Source\\TestMeshes\\MandolorianHelmet\\Mando_Helm_Mat_baseColor.png");

	Ruya::Texture normalTexture = Ruya::LoadTexture("C:\\Users\\aalpe\\Desktop\\RENGINE\\Ruya\\Game\\Source\\TestMeshes\\MandolorianHelmet\\Mando_Helm_Mat_normal.png");

	std::shared_ptr<Ruya::RenderObject> renderObject = std::make_shared<Ruya::RenderObject>(Ruya::Engine::GetInstance().GetRenderer().CreateRenderObject(mesh, albedoTexture, normalTexture));
	meshComponent->SetRenderObject(renderObject);
}

ProjectB::~ProjectB()
{
}
