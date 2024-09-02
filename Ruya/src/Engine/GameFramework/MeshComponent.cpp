#include "MeshComponent.h"
#include "../Graphics/Mesh.h"
#include "../Engine.h"

Ruya::MeshComponent::MeshComponent() : ActorComponent()
{
}

Ruya::MeshComponent::~MeshComponent()
{
}

void Ruya::MeshComponent::Update()
{
	ActorComponent::Update();
	
	Engine::GetInstance().GetRenderer().AddToRenderQueue(mesh);
}

void Ruya::MeshComponent::SetMesh(std::shared_ptr<Mesh> mesh)
{
	this->mesh = mesh;
}

void Ruya::MeshComponent::SetMaterial(const std::shared_ptr<Material> material)
{
	this->material = material;
}
