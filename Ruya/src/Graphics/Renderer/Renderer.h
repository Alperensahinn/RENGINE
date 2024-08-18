#pragma once

struct GLFWwindow;

namespace Ruya
{
	class RVulkan;

	class Renderer
	{
	public:
		Renderer(GLFWwindow& window);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

	public:
		void DrawFrame();

	private:
		void CleanUp();

	private:
		RVulkan* pRVulkan;
	};
}