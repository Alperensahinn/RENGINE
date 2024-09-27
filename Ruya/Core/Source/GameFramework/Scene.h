#pragma once
#include <GameFramework/Entity.h>
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
		
		EntityID NewActor();
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
				componentPools.insert(std::make_pair(typeIdx, std::vector<T>()));
				componentPools[typeIdx].emplace_back(T{});
			}

			auto* newComponent = &componentPools[typeIdx].back();
			newComponent->SetActorID(id);
			newComponent->SetScene(this);

			return static_cast<T*>(newComponent);
		}

		template<typename T>
		T* GetComponent(EntityID id)
		{
			std::type_index typeIdx = typeid(T);

			auto it = componentPools.find(typeIdx);

			if (it != componentPools.end()) 
			{
				for (const auto& component : it->second) 
				{
					if (component.GetActorID() == id) 
					{
						return static_cast<T*>(component);
					}
				}
			}

			throw std::runtime_error("[GameFramework] Component not found for the specified ActorID.");
		}

		template<typename T>
		std::vector<T>& GetComponents()
		{
			std::type_index typeIdx = typeid(T);

			if (componentPools.find(typeIdx) == componentPools.end()) 
			{
				componentPools[typeIdx] = std::vector<T>();
			}

			return std::any_cast<std::vector<T>&>(componentPools[typeIdx]);
		}

	private:
		std::vector<Entity> entities;
		std::unordered_map<EntityID, Entity*> entityIDs;
		std::unordered_map<std::type_index, std::vector<std::any>> componentPools;

		std::vector<SceneSystem*> sceneSystems;
	};

}