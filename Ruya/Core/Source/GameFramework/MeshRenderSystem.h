#pragma once
#include <GameFramework/SceneSystem.h>

namespace Ruya
{
	class MeshRenderSystem : public SceneSystem
	{
	public:
		MeshRenderSystem() = default;
		~MeshRenderSystem() = default;

	public:
		void OnSceneStart(Scene& scene) override;
		void OnSceneUpdate(Scene& scene) override;
		void OnSceneDestroy(Scene& scene) override;
	};
}