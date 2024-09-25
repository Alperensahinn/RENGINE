#pragma once
#include <Utilities/Math/RMath.h>
#include <GameFramework/ActorComponent.h>

namespace Ruya
{
	class TransformComponent : public ActorComponent
	{
	public:
		TransformComponent() = default;
		~TransformComponent() = default;

	public:
		math::mat4 GetTransformMatrix();

	public:
		math::vec3 position;
		math::vec3 rotation;
		math::vec3 scale;
	};
}