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
		MeshComponent();
		~MeshComponent();

		MeshComponent(const MeshComponent&) = delete;
		MeshComponent& operator=(const MeshComponent&) = delete;

	public:
		void Start() override;
		void Update() override;

		void CleanUp() override;

		void SetRenderObject(std::shared_ptr<RenderObject> renderObject);

	private:
		bool bIsStaticMesh;
		std::shared_ptr<RenderObject> renderObject;
	};
}