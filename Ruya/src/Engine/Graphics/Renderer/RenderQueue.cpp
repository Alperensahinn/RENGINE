#include "RenderQueue.h"

Ruya::RenderQueue::RenderQueue()
{
}

Ruya::RenderQueue::~RenderQueue()
{
}

std::shared_ptr<Ruya::Mesh> Ruya::RenderQueue::Pop()
{
    std::shared_ptr<Mesh> frontMesh = meshQueue.front();
    meshQueue.pop();
    return frontMesh;
}

void Ruya::RenderQueue::Push(std::shared_ptr<Mesh> mesh)
{
    meshQueue.push(mesh);
}

bool Ruya::RenderQueue::IsEmpty()
{
    return meshQueue.empty();
}
