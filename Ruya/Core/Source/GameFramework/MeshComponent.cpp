#include "MeshComponent.h"
#include "../Graphics/Mesh.h"
#include "../Engine.h"
#include <GameFramework/Scene.h>
#include <GameFramework/TransformComponent.h>

void Ruya::MeshComponent::OnSceneStart()
{
	renderObject->modelMatrix = GetScene()->GetComponent<TransformComponent>(GetActorID()).GetTransformMatrix();
}

void Ruya::MeshComponent::OnSceneUpdate()
{
	if(bIsStaticMesh == false)
	{
		renderObject->modelMatrix = GetScene()->GetComponent<TransformComponent>(GetActorID()).GetTransformMatrix();
	}

	Engine::GetInstance().GetRenderer().AddToRenderQueue(renderObject);
}

void Ruya::MeshComponent::OnSceneDestroy()
{
	renderObject->Destroy();
}

void Ruya::MeshComponent::SetRenderObject(std::shared_ptr<RenderObject> renderObject)
{
	this->renderObject = renderObject;
}

void Ruya::MeshComponent::SetIsStaticMesh(bool b)
{
	bIsStaticMesh = b;
}
