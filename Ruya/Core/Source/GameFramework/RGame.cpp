#include "RGame.h"

Ruya::RGame::RGame()
{
	InitAvaibleSceneIDs();
}

Ruya::RGame::~RGame()
{
}

void Ruya::RGame::Start()
{
	for (const auto& pair : sceneMap)
	{
		pair.second->Start();
	}
}

void Ruya::RGame::Update()
{
	for (const auto& pair : sceneMap)
	{
		pair.second->Update();
	}
}

std::shared_ptr<Ruya::Scene> Ruya::RGame::AddScene(std::shared_ptr<Scene> scene)
{
	unsigned int newID = avaibleSceneIDs.front();
	avaibleSceneIDs.pop();
	avaibleSceneIDs.push(maxAvaibleSceneID + 1);
	maxAvaibleSceneID++;

	sceneMap.insert(std::make_pair(newID, std::move(scene)));

	return sceneMap.find(newID)->second;
}

void Ruya::RGame::RemoveScene(unsigned int sceneID)
{
	sceneMap.erase(sceneID);
	avaibleSceneIDs.push(sceneID);
}

std::unordered_map<unsigned int, std::shared_ptr<Ruya::Scene>>& Ruya::RGame::GetScenes()
{
	return sceneMap;
}

void Ruya::RGame::InitAvaibleSceneIDs()
{
	avaibleSceneIDs.push(0);
	maxAvaibleSceneID = 0;
}

void Ruya::RGame::CleanUp()
{
	for (auto& pair : sceneMap) 
	{
		if (pair.second) 
		{
			pair.second->CleanUp();
		}
	}
}
