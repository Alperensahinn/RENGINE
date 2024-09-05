#pragma once
#include "../Mesh.h"
#include <queue>
#include <memory>
#include "Drawable.h"

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
		std::shared_ptr<IDrawable> Pop();
		void Push(std::shared_ptr<IDrawable> mesh);

		bool IsEmpty();;

	private:
		std::queue<std::shared_ptr<IDrawable>> meshQueue;
	};
}