#pragma once

namespace Ruya
{
	enum class RObjectType
	{
		ROBJECT,
		GAMEOBJECT
	};

	class RObject
	{
	public:
		RObject(RObjectType type);
		~RObject();

		RObject(const RObject&) = delete;
		RObject& operator=(const RObject&) = delete;

	public:
		RObjectType GetRObjectType();

	protected:
		RObjectType rObjectType;
	};
}