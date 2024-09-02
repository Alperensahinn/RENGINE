#include "ProjectB.h"
#include "../Engine/Scene/Scene.h"
#include "../Engine/GameFramework/Actor.h"
#include "../Engine/GameFramework/MeshComponent.h"
#include "../Engine/Utilities/FileSystem/FileSystem.h"

ProjectB::ProjectB()
{
	std::unique_ptr<Ruya::Scene>& mainScene = AddScene(std::make_unique<Ruya::Scene>());

	std::unique_ptr<Ruya::Actor>& monkey = mainScene->AddActor(std::make_unique<Ruya::Actor>());

	std::unique_ptr<Ruya::ActorComponent>& monkeyMesh = monkey->AddComponent(std::make_unique<Ruya::MeshComponent>());

	Ruya::MeshComponent* meshComponent = dynamic_cast<Ruya::MeshComponent*>(monkeyMesh.get());
	meshComponent->SetMesh(Ruya::ImportFBXMesh("C:\\Users\\aalpe\\Desktop\\RENGINE\\Ruya\\src\\Engine\\TestMeshes\\Monkey.glb"));
}

ProjectB::~ProjectB()
{
}
