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
		void AddSceneSystem()
		{
			sceneSystems.push_back(new T());
		}

		template<typename T>
		T& AddComponent(EntityID id)
		{
			std::type_index typeIdx = typeid(T);
			auto it = componentPools.find(typeIdx);

			if (it == componentPools.end())
			{
				auto* componentVector = new std::vector<T>();
				componentPools[typeIdx] = static_cast<void*>(componentVector);
				it = componentPools.find(typeIdx);
			}

			auto* componentVector = static_cast<std::vector<T>*>(it->second);

			componentVector->emplace_back(T{});
			T& newComponent = componentVector->back();

			newComponent.parentID = id;

			return newComponent;
		}

		template<typename T>
		T& GetComponent(EntityID id)
		{
			std::type_index typeIdx = typeid(T);
			auto it = componentPools.find(typeIdx);

			if (it != componentPools.end())
			{
				auto* componentVector = static_cast<std::vector<T>*>(it->second);

				for (auto& component : *componentVector)
				{
					EntityComponent* baseComponent = static_cast<EntityComponent*>(&component);

					if (baseComponent->parentID == id)
					{
						return component;
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
				auto* componentVector = new std::vector<T>();
				componentPools[typeIdx] = static_cast<void*>(componentVector);
			}

			auto* typedVector = static_cast<std::vector<T>*>(componentPools[typeIdx]);

			return *typedVector;
		}

	private:
		std::vector<Entity> entities;
		std::unordered_map<EntityID, Entity*> entityIDs;
		std::unordered_map<std::type_index, void*> componentPools;

		std::vector<SceneSystem*> sceneSystems;
	};

}