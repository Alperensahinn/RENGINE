#pragma once

namespace Ruya
{
	class RObject
	{
	public:
		RObject();
		~RObject();

		RObject(const RObject&) = delete;
		RObject& operator=(const RObject&) = delete;
	};
}