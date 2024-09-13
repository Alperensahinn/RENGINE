#pragma once
#include "RObject.h"
#include "ActorComponent.h"
#include "../Utilities/Math/RMath.h"
#include <vector>
#include <memory>
#include <string>

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

		void CleanUp();

		void SetID(unsigned int id);
		unsigned int GetID();

		std::unique_ptr<ActorComponent>& AddComponent(std::unique_ptr<ActorComponent> component);
		void RemoveComponent(std::unique_ptr<ActorComponent>& component);

		void SetUpdateFunctionEnable(bool b);
		bool GetUpdateFunctionEnabled();

		void SetStatic(bool b);
		bool IsStatic();

		math::mat4 GetWorldMatrix();

	public:
		std::string name;
		Transform transform;

	private:
		math::mat4 worldMatrix;

	private:
		unsigned int id;
		bool bCanEverUpdate;
		bool bIsStatic;
		std::vector<std::unique_ptr<ActorComponent>> components;
		std::vector <std::unique_ptr<ActorComponent>> updateFunctionEnabledComponents;
		std::vector<std::shared_ptr<Actor>> childActors;
	};
}