#pragma once
#include "../GameFramework/Actor.h"
#include <queue>

namespace Ruya
{
	class Scene
	{
	public:
		Scene();
		~Scene();

		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;

	public:
		void Start();
		void Update();
		
		void CleanUp();
	public:
		std::shared_ptr<Actor> AddActor(std::shared_ptr<Actor> actor);
		void RemoveActor(unsigned int actorID);
		std::unordered_map<unsigned int, std::shared_ptr<Actor>>& GetActors();

	private:
		void InitAvaibleActorIDs();
		void InitUpdateFunctionEnabledActors();

	private:
		std::unordered_map<unsigned int, std::shared_ptr<Actor>> actorMap;
		std::queue<unsigned int> avaibleActorIDs;
		unsigned int maxAvaibleActorID;
		std::vector<unsigned int> updateFunctionEnabledActors;
	};
}