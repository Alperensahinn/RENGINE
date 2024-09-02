#pragma once
#include "../Scene/Scene.h"
#include <unordered_map>
#include <memory>

namespace Ruya
{
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

	public:
		std::unique_ptr<Scene>& AddScene(std::unique_ptr<Scene>& scene);
		void RemoveScene(unsigned int sceneID);

	private:
		void InitAvaibleSceneIDs();

	private:
		std::unordered_map<unsigned int, std::unique_ptr<Scene>> sceneMap;
		std::queue<unsigned int> avaibleSceneIDs;
		unsigned int maxAvaibleSceneID;
	};
}