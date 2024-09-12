#pragma once
#include "../Scene/Scene.h"
#include <unordered_map>
#include <memory>

namespace Ruya
{
	class Engine;

	class RGame
	{
	public:
		RGame();
		virtual ~RGame();

		RGame(const RGame&) = delete;
		RGame& operator=(const RGame&) = delete;

	public:
		void Start();
		void Update();

		void CleanUp();
	public:
		std::shared_ptr<Scene> AddScene(std::shared_ptr<Scene> scene);
		void RemoveScene(unsigned int sceneID);
		std::unordered_map<unsigned int, std::shared_ptr<Scene>>& GetScenes();

	private:
		void InitAvaibleSceneIDs();

	private:
		std::unordered_map<unsigned int, std::shared_ptr<Scene>> sceneMap;
		std::queue<unsigned int> avaibleSceneIDs;
		unsigned int maxAvaibleSceneID;
	};
}