#pragma once
#include "../Mesh.h"
#include <queue>
#include <memory>
#include "Drawable.h"
#include <Graphics/Vulkan/RVulkan.h>
#include "RenderObject.h"

namespace Ruya
{
	class RenderQueue
	{
	public:
		RenderQueue();
		virtual ~RenderQueue();

		RenderQueue(const RenderQueue&) = delete;
		RenderQueue& operator=(const RenderQueue&) = delete;

	public:
		std::shared_ptr<RenderObject> Pop();
		void Push(std::shared_ptr<RenderObject> renderObject);

		bool IsEmpty();;

	private:
		std::queue<std::shared_ptr<RenderObject>> renderQueue;
	};
}