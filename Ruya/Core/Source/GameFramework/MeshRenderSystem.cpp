#include "MeshRenderSystem.h"
#include <GameFramework/MeshComponent.h>
#include <GameFramework/TransformComponent.h>

void Ruya::MeshRenderSystem::OnSceneStart(Scene& scene)
{
	for(const MeshComponent& component : scene.GetComponents<MeshComponent>())
	{
		TransformComponent& transform = scene.GetComponent<TransformComponent>(component.parentID);

		math::mat4 worldMatrix = math::mat4(1.0f);
		worldMatrix = glm::translate(worldMatrix, transform.position);
		worldMatrix = glm::scale(worldMatrix, transform.scale);

		component.renderObject->modelMatrix = worldMatrix;
	}
}

void Ruya::MeshRenderSystem::OnSceneUpdate(Scene& scene)
{
	for (const MeshComponent& component : scene.GetComponents<MeshComponent>())
	{
		if (component.bIsStaticMesh == false)
		{
			TransformComponent& transform = scene.GetComponent<TransformComponent>(component.parentID);

			math::mat4 worldMatrix = math::mat4(1.0f);
			worldMatrix = glm::translate(worldMatrix, transform.position);
			worldMatrix = glm::scale(worldMatrix, transform.scale);

			component.renderObject->modelMatrix = worldMatrix;
		}

		Engine::GetInstance().GetRenderer().AddToRenderQueue(component.renderObject);
	}
}

void Ruya::MeshRenderSystem::OnSceneDestroy(Scene& scene)
{
	for (const MeshComponent& component : scene.GetComponents<MeshComponent>())
	{
		component.renderObject->Destroy();
	}
}
