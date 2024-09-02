#include "ActorComponent.h"

Ruya::ActorComponent::ActorComponent() : bCanEverUpdate(true)
{
}

Ruya::ActorComponent::~ActorComponent()
{
}

void Ruya::ActorComponent::Start()
{
}

void Ruya::ActorComponent::Update()
{

}

void Ruya::ActorComponent::SetUpdateFunctionEnable(bool b)
{
	bCanEverUpdate = b;
}

bool Ruya::ActorComponent::GetUpdateFunctionEnabled()
{
	return bCanEverUpdate;
}
