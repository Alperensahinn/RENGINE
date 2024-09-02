#pragma once

namespace Ruya
{
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

		void SetUpdateFunctionEnable(bool b);
		bool GetUpdateFunctionEnabled();

	private:
		bool bCanEverUpdate;
	};
}