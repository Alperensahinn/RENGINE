#include "RenderQueue.h"


Ruya::RenderQueue::RenderQueue()
{
}

Ruya::RenderQueue::~RenderQueue()
{
}

std::shared_ptr<Ruya::RenderObject> Ruya::RenderQueue::Pop()
{
    std::shared_ptr<RenderObject> renderObject = renderQueue.front();
    renderQueue.pop();
    return renderObject;
}

void Ruya::RenderQueue::Push(std::shared_ptr<RenderObject> renderObject)
{
    renderQueue.push(renderObject);
}

bool Ruya::RenderQueue::IsEmpty()
{
    return renderQueue.empty();
}
