#pragma once
#include <memory>

namespace Ruya
{
	class Actor;

	class ActorComponent
	{
	public:
		ActorComponent();
		~ActorComponent();

		ActorComponent(const ActorComponent&) = delete;
		ActorComponent& operator=(const ActorComponent&) = delete;

	public:
		virtual void Start();
		virtual void Update();

		virtual void CleanUp();

		void SetUpdateFunctionEnable(bool b);
		bool GetUpdateFunctionEnabled();

		void SetActor(Actor* actor);
		Actor& GetActor();

	private:
		bool bCanEverUpdate;
		Actor* actor;
	};
}