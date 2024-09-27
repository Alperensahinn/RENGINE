#pragma once

namespace Ruya
{
	class Scene;

	enum class UpdateType
	{
		SceneUpdate,
		GameUpdate,
		SceneAndGameUpdate,
		NoUpdate
	};

	class SceneSystem
	{
	public:
		SceneSystem() = default;
		virtual ~SceneSystem() = default;

	public:
		virtual void OnSceneStart(Scene& scene);
		virtual void OnSceneUpdate(Scene& scene);
		virtual void OnSceneDestroy(Scene& scene);

		virtual void OnGameStart(Scene& scene);
		virtual void OnGameUpdate(Scene& scene);
		virtual void OnGameDestroy(Scene& scene);

		void SetUpdateType(UpdateType type);
		UpdateType GetUpdateType();

	private:
		UpdateType updateType = UpdateType::SceneAndGameUpdate;
	};
}