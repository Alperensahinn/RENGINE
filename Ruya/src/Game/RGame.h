#pragma once
#include "RObject.h"
#include <string>
#include <vector>
#include <memory>

namespace Ruya
{
	struct RGameCreateInfo
	{
		std::string gameName;

		struct
		{
			uint32_t windowWidht;
			uint32_t windowHeight;
		}window;

	};

	class RGame
	{
	public:
		RGame(RGameCreateInfo createInfo);
		~RGame();

		RGame(const RGame&) = delete;
		RGame& operator=(const RGame&) = delete;

	public:
		void Init();
		void Start();
		void Process();

		void AddRObject(std::unique_ptr<RObject>& rObject);

	private:
		std::vector<std::unique_ptr<RObject>> rObjects;
	};
}