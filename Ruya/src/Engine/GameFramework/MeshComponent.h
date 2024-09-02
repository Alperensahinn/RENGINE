#pragma once
#include "ActorComponent.h"

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
		void Update() override;

		void SetMesh(const std::shared_ptr<Mesh> mesh);
		void SetMaterial(const std::shared_ptr<Material> material);

	private:
		bool bIsStaticMesh;
		std::shared_ptr<Mesh> mesh;
		std::shared_ptr<Material> material;
	};
}