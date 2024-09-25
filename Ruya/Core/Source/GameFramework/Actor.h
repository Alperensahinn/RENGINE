#pragma once

#include <Utilities/Math/RMath.h>

#include <bitset>
#include <string>


namespace Ruya
{
	typedef std::uint64_t ActorID;

	struct Actor
	{
		ActorID id;
		std::string name;
	};
}
