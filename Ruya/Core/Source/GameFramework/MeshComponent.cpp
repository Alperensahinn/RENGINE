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
}

void Ruya::MeshComponent::Update()
{
	ActorComponent::Update();
	
	Engine::GetInstance().GetRenderer().AddToRenderQueue(renderObject);
}

void Ruya::MeshComponent::CleanUp()
{
	ActorComponent::CleanUp();

	renderObject->Destroy();
}

void Ruya::MeshComponent::SetRenderObject(std::shared_ptr<RenderObject> renderObject)
{
	this->renderObject = renderObject;
}