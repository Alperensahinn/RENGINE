#include "ProjectB.h"
#include <Scene/Scene.h>
#include <GameFramework/Actor.h>
#include <GameFramework/MeshComponent.h>
#include <Utilities/FileSystem/FileSystem.h>

ProjectB::ProjectB()
{
	std::unique_ptr<Ruya::Scene>& mainScene = AddScene(std::make_unique<Ruya::Scene>());

	std::unique_ptr<Ruya::Actor>& monkey = mainScene->AddActor(std::make_unique<Ruya::Actor>());

	std::unique_ptr<Ruya::ActorComponent>& monkeyMesh = monkey->AddComponent(std::make_unique<Ruya::MeshComponent>());

	Ruya::MeshComponent* meshComponent = dynamic_cast<Ruya::MeshComponent*>(monkeyMesh.get());
	meshComponent->SetMesh(Ruya::ImportFBXMesh("C:\\Users\\aalpe\\Desktop\\RENGINE\\Ruya\\Game\\Source\\TestMeshes\\Monkey.glb"));
}

ProjectB::~ProjectB()
{
}
