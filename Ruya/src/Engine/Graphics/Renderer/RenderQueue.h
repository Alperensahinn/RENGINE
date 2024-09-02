#pragma once
#include "../Mesh.h"
#include <queue>
#include <memory>

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
		std::shared_ptr<Mesh> Pop();
		void Push(std::shared_ptr<Mesh> mesh);

		bool IsEmpty();;

	private:
		std::queue<std::shared_ptr<Mesh>> meshQueue;
	};
}