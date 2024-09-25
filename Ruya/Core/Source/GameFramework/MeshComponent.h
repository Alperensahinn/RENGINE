#pragma once
#include "ActorComponent.h"

#include <Graphics/Renderer/RenderObject.h>

#include <memory>

namespace Ruya
{
	struct Mesh;
	struct Material;

	class MeshComponent : public ActorComponent
	{
	public:
		MeshComponent() = default;
		~MeshComponent() = default;

	public:
		void OnSceneStart() override;
		void OnSceneUpdate() override;
		void OnSceneDestroy() override;

		void SetRenderObject(std::shared_ptr<RenderObject> renderObject);
		void SetIsStaticMesh(bool b);

	private:
		bool bIsStaticMesh = true;
		std::shared_ptr<RenderObject> renderObject;
	};
}