#pragma once
#include <memory>

namespace Ruya
{
	typedef std::uint64_t ActorID;
	class Scene;

	enum class UpdateType
	{
		SceneUpdate,
		GameUpdate,
		SceneAndGameUpdate,
		NoUpdate
	};

	class ActorComponent
	{
	public:
		ActorComponent() = default;
		~ActorComponent() = default;

	public:
		virtual void OnSceneStart();
		virtual void OnSceneUpdate();
		virtual void OnSceneDestroy();

		virtual void OnGameStart();
		virtual void OnGameUpdate();
		virtual void OnGameDestroy();

		void SetUpdateType(UpdateType type);
		bool GetUpdateType();

		void SetActorID(ActorID id);
		ActorID GetActorID();

		void SetScene(Scene* scene);
		Scene* GetScene();

	private:
		UpdateType updateType = UpdateType::SceneAndGameUpdate;
		ActorID actorID;
		Scene* scene;
	};
}