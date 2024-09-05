#include "RenderQueue.h"

Ruya::RenderQueue::RenderQueue()
{
}

Ruya::RenderQueue::~RenderQueue()
{
}

std::shared_ptr<Ruya::IDrawable> Ruya::RenderQueue::Pop()
{
    std::shared_ptr<IDrawable> frontMesh = meshQueue.front();
    meshQueue.pop();
    return frontMesh;
}

void Ruya::RenderQueue::Push(std::shared_ptr<IDrawable> mesh)
{
    meshQueue.push(mesh);
}

bool Ruya::RenderQueue::IsEmpty()
{
    return meshQueue.empty();
}
