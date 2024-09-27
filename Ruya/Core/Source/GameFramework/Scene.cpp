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
		system->OnSceneStart();
	}
}

void Ruya::Scene::OnSceneUpdate()
{
	for (SceneSystem* system : sceneSystems)
	{
		system->OnSceneUpdate();
	}
}

void Ruya::Scene::OnSceneDestroy()
{
	for (SceneSystem* system : sceneSystems)
	{
		system->OnSceneDestroy();
	}
}

void Ruya::Scene::OnGameStart()
{
	for (SceneSystem* system : sceneSystems)
	{
		system->OnGameStart();
	}
}

void Ruya::Scene::OnGameUpdate()
{
	for (SceneSystem* system : sceneSystems)
	{
		system->OnGameUpdate();
	}
}

void Ruya::Scene::OnGameDestroy()
{
	for (SceneSystem* system : sceneSystems)
	{
		system->OnGameDestroy();
	}
}

Ruya::EntityID Ruya::Scene::NewActor()
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


std::vector<Ruya::Entity>& Ruya::Scene::GetActors()
{
	return entities;
}
