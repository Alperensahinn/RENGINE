#include "Scene.h"

Ruya::Scene::Scene()
{

}

Ruya::Scene::~Scene()
{
}

void Ruya::Scene::OnSceneStart()
{
	for (auto it = componentPools.begin(); it != componentPools.end(); ++it) 
	{
		for (ActorComponent& component : it->second)
		{
			component.OnSceneStart();
		}
	}
}

void Ruya::Scene::OnSceneUpdate()
{
	for (auto it = componentPools.begin(); it != componentPools.end(); ++it)
	{
		for (ActorComponent& component : it->second)
		{
			component.OnSceneUpdate();
		}
	}
}

void Ruya::Scene::OnSceneDestroy()
{
	for (auto it = componentPools.begin(); it != componentPools.end(); ++it)
	{
		for (ActorComponent& component : it->second)
		{
			component.OnSceneDestroy();
		}
	}
}

void Ruya::Scene::OnGameStart()
{
	for (auto it = componentPools.begin(); it != componentPools.end(); ++it)
	{
		for (ActorComponent& component : it->second)
		{
			component.OnGameStart();
		}
	}
}

void Ruya::Scene::OnGameUpdate()
{
	for (auto it = componentPools.begin(); it != componentPools.end(); ++it)
	{
		for (ActorComponent& component : it->second)
		{
			component.OnGameUpdate();
		}
	}
}

void Ruya::Scene::OnGameDestroy()
{
	for (auto it = componentPools.begin(); it != componentPools.end(); ++it)
	{
		for (ActorComponent& component : it->second)
		{
			component.OnGameDestroy();
		}
	}
}

Ruya::ActorID Ruya::Scene::NewActor()
{
	actors.push_back({ actors.size(), "New Actor"});
	return ActorID();
}
