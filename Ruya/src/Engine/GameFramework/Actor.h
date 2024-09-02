#pragma once
#include "RObject.h"
#include "ActorComponent.h"
#include "../Utilities/Math/RMath.h"
#include <vector>
#include <memory>

namespace Ruya
{
	struct Transform
	{
		math::vec3 position;
		math::vec3 rotation;
		math::vec3 scale;
	};

	class Actor : public RObject
	{
	public:
		Actor();
		virtual ~Actor();

		Actor(const Actor&) = delete;
		Actor& operator=(const Actor&) = delete;

	public:
		void Start();
		void Update();

		void SetID(unsigned int id);
		unsigned int GetID();

		std::unique_ptr<ActorComponent>& AddComponent(std::unique_ptr<ActorComponent> component);
		void RemoveComponent(std::unique_ptr<ActorComponent>& component);

		void SetUpdateFunctionEnable(bool b);
		bool GetUpdateFunctionEnabled();

	public:
		Transform transform;

	private:
		unsigned int id;
		bool bCanEverUpdate;
		std::vector<std::unique_ptr<ActorComponent>> components;
		std::vector <std::unique_ptr<ActorComponent>> updateFunctionEnabledComponents;
	};
}