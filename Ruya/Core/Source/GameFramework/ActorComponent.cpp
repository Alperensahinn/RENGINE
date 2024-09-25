#include "ActorComponent.h"

void Ruya::ActorComponent::OnSceneStart()
{
}

void Ruya::ActorComponent::OnSceneUpdate()
{
}

void Ruya::ActorComponent::OnSceneDestroy()
{
}

void Ruya::ActorComponent::OnGameStart()
{
}

void Ruya::ActorComponent::OnGameUpdate()
{
}

void Ruya::ActorComponent::OnGameDestroy()
{
}

void Ruya::ActorComponent::SetUpdateType(UpdateType type)
{
}

bool Ruya::ActorComponent::GetUpdateType()
{
	return false;
}

void Ruya::ActorComponent::SetActorID(ActorID id)
{
}

Ruya::ActorID Ruya::ActorComponent::GetActorID()
{
	return actorID;
}

void Ruya::ActorComponent::SetScene(Scene* scene)
{
	this->scene = scene;
}

Ruya::Scene* Ruya::ActorComponent::GetScene()
{
	return scene;
}
