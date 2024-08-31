#include "RGame.h"
#include "GameObject.h"

Ruya::RGame::RGame(RGameCreateInfo createInfo)
{

}

Ruya::RGame::~RGame()
{

}

void Ruya::RGame::Init()
{
    for (std::unique_ptr<RObject>& rObject : rObjects)
    {
        if (rObject->GetRObjectType() == RObjectType::GAMEOBJECT)
        {
            static_cast<GameObject*>(rObject.get())->Init();
        }
    }
}

void Ruya::RGame::Start()
{
    for (std::unique_ptr<RObject>& rObject : rObjects)
    {
        if (rObject->GetRObjectType() == RObjectType::GAMEOBJECT)
        {
            static_cast<GameObject*>(rObject.get())->Start();
        }
    }
}

void Ruya::RGame::Process()
{
    for (std::unique_ptr<RObject>& rObject : rObjects)
    {
        if(rObject->GetRObjectType() == RObjectType::GAMEOBJECT)
        {
            static_cast<GameObject*>(rObject.get())->Update();
        }
    }
}

void Ruya::RGame::AddRObject(std::unique_ptr<RObject>& rObject)
{
	rObjects.push_back(std::move(rObject));
}

