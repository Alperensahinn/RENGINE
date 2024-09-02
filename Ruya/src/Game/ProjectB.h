#pragma once
#include "../Engine/GameFramework/RGame.h"

class ProjectB : public Ruya::RGame
{
public:
	ProjectB();
	virtual ~ProjectB();

	ProjectB(const ProjectB&) = delete;
	ProjectB& operator=(const ProjectB&) = delete;
};
