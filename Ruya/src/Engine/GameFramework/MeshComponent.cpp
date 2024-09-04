#include "MeshComponent.h"
#include "../Graphics/Mesh.h"
#include "../Engine.h"


Ruya::MeshComponent::MeshComponent() : ActorComponent()
{
}

Ruya::MeshComponent::~MeshComponent()
{
}

void Ruya::MeshComponent::Start()
{
	ActorComponent::Start();
	drawable = Engine::GetInstance().GetRenderer().LoadMesh(mesh);
}

void Ruya::MeshComponent::Update()
{
	ActorComponent::Update();
	
	Engine::GetInstance().GetRenderer().AddToRenderQueue(drawable);
}

void Ruya::MeshComponent::SetMesh(std::shared_ptr<Mesh> mesh)
{
	this->mesh = mesh;
}

void Ruya::MeshComponent::SetMaterial(const std::shared_ptr<Material> material)
{
	this->material = material;
}
