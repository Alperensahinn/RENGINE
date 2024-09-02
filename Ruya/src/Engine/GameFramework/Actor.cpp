#include "Actor.h"

Ruya::Actor::Actor() : RObject()
{    
    behaviorType = BehaviorType::Static;
}

Ruya::Actor::~Actor()
{
}

void Ruya::Actor::Start()
{
    for(std::unique_ptr<ActorComponent>& component : components)
    {
        if(component->GetUpdateFunctionEnabled())
        {
            updateFunctionEnabledComponents.push_back(std::move(component));
        }
    }
}

void Ruya::Actor::Update()
{
    for (std::unique_ptr<ActorComponent>& component : updateFunctionEnabledComponents)
    {
        component->Update();
    }
}

void Ruya::Actor::SetID(unsigned int id)
{
    this->id = id;
}

unsigned int Ruya::Actor::GetID()
{
    return id;
}

std::unique_ptr<Ruya::ActorComponent>& Ruya::Actor::AddComponent(std::unique_ptr<ActorComponent> component)
{
	components.push_back(std::move(component));
	return components.back();
}

void Ruya::Actor::RemoveComponent(std::unique_ptr<ActorComponent>& component)
{
    for (auto it = components.begin(); it != components.end(); ++it)
    {
        if ((*it).get() == component.get())
        {
            components.erase(it);
            return;
        }
    }
}

void Ruya::Actor::SetUpdateFunctionEnable(bool b)
{
    bCanEverUpdate = b;
}

bool Ruya::Actor::GetUpdateFunctionEnabled()
{
    return bCanEverUpdate;
}

void Ruya::Actor::SetBehaviorType(BehaviorType type)
{
    behaviorType = type;
}

Ruya::Actor::BehaviorType Ruya::Actor::GetBehaviorType()
{
    return behaviorType;
}
