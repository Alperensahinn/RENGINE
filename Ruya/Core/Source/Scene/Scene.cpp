#include "Scene.h"

Ruya::Scene::Scene()
{
	InitAvaibleActorIDs();
}

Ruya::Scene::~Scene()
{
}

void Ruya::Scene::Start()
{
	InitUpdateFunctionEnabledActors();

	for (const auto& pair : actorMap)
	{
		pair.second->Start();
	}
}

void Ruya::Scene::Update()
{
	for (const auto& actor : updateFunctionEnabledActors)
	{
		actor->Update();
	}
}

void Ruya::Scene::CleanUp()
{
	for (auto& pair : actorMap)
	{
		if (pair.second)
		{
			pair.second->CleanUp();
		}
	}
}

void Ruya::Scene::InitAvaibleActorIDs()
{
	avaibleActorIDs.push(0);
	maxAvaibleActorID = 0;
}

void Ruya::Scene::InitUpdateFunctionEnabledActors()
{
	for (const auto& pair : actorMap)
	{
		const auto& actor = pair.second;
		if (actor->GetUpdateFunctionEnabled())
		{
			updateFunctionEnabledActors.push_back(actor);
		}
	}
}

std::shared_ptr<Ruya::Actor> Ruya::Scene::AddActor(std::shared_ptr<Actor> actor)
{
	unsigned int newID = avaibleActorIDs.front();
	avaibleActorIDs.pop();
	avaibleActorIDs.push(maxAvaibleActorID + 1);
	maxAvaibleActorID++;

	actorMap.insert(std::make_pair(newID, std::move(actor)));

	return actorMap.find(newID)->second;
}

void Ruya::Scene::RemoveActor(unsigned int actorID)
{
	actorMap.erase(actorID);
	avaibleActorIDs.push(actorID);
}

std::unordered_map<unsigned int, std::shared_ptr<Ruya::Actor>>& Ruya::Scene::GetActors()
{
	return actorMap;
}
