#pragma once
#include "RObject.h"

namespace Ruya
{
	class Actor : public RObject
	{
	public:
		Actor();
		~Actor();

		Actor(const Actor&) = delete;
		Actor& operator=(const Actor&) = delete;

	public:

	};
}