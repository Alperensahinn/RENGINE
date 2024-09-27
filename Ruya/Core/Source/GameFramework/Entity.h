#pragma once
#include <string>

namespace Ruya
{
	typedef std::uint64_t EntityID;

	struct Entity
	{
		EntityID id;
		std::string name;
	};
}