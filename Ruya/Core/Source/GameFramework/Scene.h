#pragma once
#include <GameFramework/Entity.h>
#include <GameFramework/EntityComponent.h>
#include <GameFramework/SceneSystem.h>

#include <queue>
#include <typeindex>
#include <stdexcept>
#include <any>
#include <unordered_map>

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
		void OnSceneStart();
		void OnSceneUpdate();
		void OnSceneDestroy();

		void OnGameStart();
		void OnGameUpdate();
		void OnGameDestroy();
		
		EntityID NewEntity();
		Entity* GetEntity(EntityID id);
		std::vector<Entity>& GetEntities();

		template<typename T>
		T* AddComponent(EntityID id)
		{
			std::type_index typeIdx = typeid(T);
			auto it = componentPools.find(typeIdx);

			if (it != componentPools.end())
			{
				it->second.emplace_back(T{}); 
			}

			else
			{
				std::vector<std::any> anyVector;
				anyVector.emplace_back(T{});
				componentPools.insert(std::make_pair(typeIdx, anyVector));
			}

			EntityComponent* newComponentPtr = std::any_cast<EntityComponent>(&componentPools[typeIdx].back());
			newComponentPtr->parentID = id;
			return static_cast<T*>(newComponentPtr);
		}

		template<typename T>
		T* GetComponent(EntityID id)
		{
			std::type_index typeIdx = typeid(T);
			auto it = componentPools.find(typeIdx);

			if (it != componentPools.end())
			{
				for (auto& component : it->second)
				{
					EntityComponent* c = std::any_cast<EntityComponent>(&component);
					if (c && c->parentID == id)
					{
						return static_cast<T*>(c);
					}
				}
			}

			throw std::runtime_error("[GameFramework] Component not found for the specified ActorID.");
		}

		template<typename T>
		std::vector<T>* GetComponents()
		{

			std::type_index typeIdx = typeid(T);

			if (componentPools.find(typeIdx) == componentPools.end())
			{
				componentPools[typeIdx] = std::vector<std::any>();
			}

			return nullptr;
		}

	private:
		std::vector<Entity> entities;
		std::unordered_map<EntityID, Entity*> entityIDs;
		std::unordered_map<std::type_index, std::vector<std::any>> componentPools;

		std::vector<SceneSystem*> sceneSystems;
	};

}