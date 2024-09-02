#include "Scene.h"

Ruya::Scene::Scene()
{
}

Ruya::Scene::~Scene()
{
}

void Ruya::Scene::Start()
{
	InitAvaibleActorIDs();
	InitUpdateFunctionEnabledActors();
}

void Ruya::Scene::Update()
{
	for (unsigned int& actorID : updateFunctionEnabledActors)
	{
		auto it = actorMap.find(actorID);

		if (it != actorMap.end())
		{
			std::unique_ptr<Actor>& actor = it->second;
			actor->Update();
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
			updateFunctionEnabledActors.push_back(actor->GetID());
		}
	}
}

std::unique_ptr<Ruya::Actor>& Ruya::Scene::AddActor(std::unique_ptr<Actor>& actor)
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
