#pragma once
#include <GameFramework/EntityComponent.h>
#include <Graphics/Renderer/RenderObject.h>
#include <memory>

namespace Ruya
{
	struct MeshComponent : public EntityComponent
	{
		bool bIsStaticMesh = true;
		std::shared_ptr<RenderObject> renderObject;
	};
}