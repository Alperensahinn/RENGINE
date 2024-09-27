#pragma once
#include <GameFramework/EntityComponent.h>
#include <Utilities/Math/RMath.h>

namespace Ruya
{
	struct TransformComponent : public EntityComponent
	{
		math::vec3 position = math::vec3(0.0f, 0.0f, 0.0f);
		math::vec3 rotation = math::vec3(0.0f, 0.0f, 0.0f);;
		math::vec3 scale = math::vec3(1.0f, 1.0f, 1.0f);;
	};
}