#pragma once

namespace Ruya
{
	class RObject
	{
	public:
		RObject();
		virtual ~RObject();

		RObject(const RObject&) = delete;
		RObject& operator=(const RObject&) = delete;
	};
}