#pragma once
#include "RObject.h"

namespace Ruya
{
	class GameObject : public RObject
	{
	public:
		GameObject();
		~GameObject();

		GameObject(const GameObject&) = delete;
		GameObject& operator=(const GameObject&) = delete;

	public:
		virtual void Init();
		virtual void Start();
		virtual void Update();
	};
}