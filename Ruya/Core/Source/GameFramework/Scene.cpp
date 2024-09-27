#include "Scene.h"

Ruya::Scene::Scene()
{

}

Ruya::Scene::~Scene()
{
}

void Ruya::Scene::OnSceneStart()
{
	for (SceneSystem* system : sceneSystems) 
	{
		system->OnSceneStart(*this);
	}
}

void Ruya::Scene::OnSceneUpdate()
{
	for (SceneSystem* system : sceneSystems)
	{
		system->OnSceneUpdate(*this);
	}
}

void Ruya::Scene::OnSceneDestroy()
{
	for (SceneSystem* system : sceneSystems)
	{
		system->OnSceneDestroy(*this);
	}
}

void Ruya::Scene::OnGameStart()
{
	for (SceneSystem* system : sceneSystems)
	{
		system->OnGameStart(*this);
	}
}

void Ruya::Scene::OnGameUpdate()
{
	for (SceneSystem* system : sceneSystems)
	{
		system->OnGameUpdate(*this);
	}
}

void Ruya::Scene::OnGameDestroy()
{
	for (SceneSystem* system : sceneSystems)
	{
		system->OnGameDestroy(*this);
	}
}

Ruya::EntityID Ruya::Scene::NewEntity()
{
	entities.push_back({ entities.size(), "New Actor"});

	Entity* newEntityRef = &entities.back();

	entityIDs.insert({ newEntityRef->id, newEntityRef });

	return newEntityRef->id;
}

Ruya::Entity* Ruya::Scene::GetEntity(EntityID id)
{
	return &entities[id];
}


std::vector<Ruya::Entity>& Ruya::Scene::GetEntities()
{
	return entities;
}
