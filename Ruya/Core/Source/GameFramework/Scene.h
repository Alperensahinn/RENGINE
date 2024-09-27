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

			if (it == componentPools.end())
			{
				std::vector<T> componentVector;
				componentPools[typeIdx] = std::move(componentVector);
				it = componentPools.find(typeIdx);
			}

			auto& componentVector = std::any_cast<std::vector<T>&>(it->second);

			componentVector.emplace_back(T{});
			T* newComponentPtr = &componentVector.back();

			newComponentPtr->parentID = id;

			return newComponentPtr;
		}

		template<typename T>
		T* GetComponent(EntityID id)
		{
			std::type_index typeIdx = typeid(T);
			auto it = componentPools.find(typeIdx);

			if (it != componentPools.end())
			{
				auto& componentVector = std::any_cast<std::vector<std::any>&>(it->second);

				for (auto& component : componentVector)
				{
					EntityComponent* baseComponent = std::any_cast<EntityComponent>(&component);

					if (baseComponent && baseComponent->parentID == id)
					{
						T* specificComponent = std::any_cast<T>(&component);

						if (specificComponent)
						{
							return specificComponent;
						}

						else
						{
							throw std::runtime_error("[GameFramework] Component type mismatch for the specified EntityID.");
						}
					}
				}
			}

			throw std::runtime_error("[GameFramework] Component not found for the specified EntityID.");
		}

		template<typename T>
		std::vector<T>& GetComponents() 
		{
			std::type_index typeIdx = typeid(T);
			if (componentPools.find(typeIdx) == componentPools.end())
			{
				componentPools[typeIdx] = std::vector<T>();
			}

			auto& typedVector = std::any_cast<std::vector<T>&>(componentPools[typeIdx]);

			return typedVector;
		}

	private:
		std::vector<Entity> entities;
		std::unordered_map<EntityID, Entity*> entityIDs;
		std::unordered_map<std::type_index, std::any> componentPools;

		std::vector<SceneSystem*> sceneSystems;
	};

}