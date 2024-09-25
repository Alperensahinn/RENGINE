#pragma once
#include <GameFramework/Actor.h>
#include <GameFramework/ActorComponent.h>
#include <queue>
#include <typeindex>

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
		
		ActorID NewActor();

		template<typename T>
		T& AddComponent(ActorID id) 
		{
			std::type_index typeIdx = typeid(T);
			auto it = componentPools.find(typeIdx);

			if (it != componentPools.end())
			{
				it->second.emplace_back(T{});
			}

			else
			{
				componentPools[typeIdx] = std::vector<T>{ T{} };
			}

			componentPools[typeIdx].back().SetActorID(id);
			componentPools[typeIdx].back().SetScene(this);

			return componentPools[typeIdx].back();
		}

		template<typename T>
		T& GetComponent(ActorID id)
		{
			std::type_index typeIdx = typeid(T);

			auto it = componentPools.find(typeIdx);

			if (it != componentPools.end()) 
			{
				for (ActorComponent& component : it->second) 
				{
					if (component.GetActorID() == id) 
					{
						return static_cast<T&>(component);
					}
				}
			}

			throw std::runtime_error("[GameFramework] Component not found for the specified ActorID.");
		}

	private:
		std::vector<Actor> actors;
		std::unordered_map<std::type_index, std::vector<ActorComponent>> componentPools;
	};

}