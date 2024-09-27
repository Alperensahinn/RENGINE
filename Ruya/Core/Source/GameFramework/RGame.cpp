#include "RGame.h"

Ruya::RGame::RGame()
{
	InitAvaibleSceneIDs();
}

Ruya::RGame::~RGame()
{
}

void Ruya::RGame::StartScene()
{
	for (const auto& pair : sceneMap)
	{
		pair.second->OnSceneStart();
	}
}

void Ruya::RGame::UpdateScene()
{
	for (const auto& pair : sceneMap)
	{
		pair.second->OnSceneUpdate();
	}
}

void Ruya::RGame::DestroyScene()
{
	for (const auto& pair : sceneMap)
	{
		pair.second->OnSceneDestroy();
	}
}

void Ruya::RGame::StartGame()
{
	for (const auto& pair : sceneMap)
	{
		pair.second->OnGameStart();
	}
}

void Ruya::RGame::UpdateGame()
{
	for (const auto& pair : sceneMap)
	{
		pair.second->OnGameUpdate();
	}
}

void Ruya::RGame::DestroyGame()
{
	for (const auto& pair : sceneMap)
	{
		pair.second->OnGameDestroy();
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
			pair.second->OnSceneDestroy();
		}
	}
}
