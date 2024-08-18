#pragma once

#include "RVulkanConfig.h"

#include <vector>
#include <deque>
#include <functional>

struct GLFWwindow;

namespace Ruya
{
	struct VKDeletionQueue
	{
		std::deque<std::function<void()>> deletors;

		void PushFunction(std::function<void()>&& function) {
			deletors.push_back(function);
		}

		void flush() {
			for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
				(*it)();
			}
			deletors.clear();
		}
	};

	struct VKFrameData
	{
		VkCommandPool commandPool = VK_NULL_HANDLE;
		VkCommandBuffer mainCommandBuffer = VK_NULL_HANDLE;

		VkSemaphore swapchainSemaphore = VK_NULL_HANDLE;;
		VkSemaphore renderSemaphore = VK_NULL_HANDLE;;
		VkFence renderFence = VK_NULL_HANDLE;;

		VKDeletionQueue deletionQueue;
	};

	struct VKAllocatedImage
	{
		VkImage image;
		VkImageView imageView;
		VmaAllocation allocation;
		VkExtent3D imageExtent;
		VkFormat imageFormat;
	};

	constexpr uint32_t frameOverlap = 2;

	class RVulkan
	{
	public:
		RVulkan(GLFWwindow& window);
		~RVulkan();

		RVulkan(const RVulkan&) = delete;
		RVulkan& operator=(const RVulkan&) = delete;

	public:
		void Init(GLFWwindow& window);
		void CleanUp();

		void Draw();
		void WaitVulkanDevice();
	private:
		void CreateInstance();
		bool CheckValidationLayerSupport();
		void CreateDebugMessenger();
		void DestroyDebugUtilsMessenger();
		void SelectPhysicalDevice();
		void CheckQueueFamilies();
		void CreateDevice();
		void SetQueues();
		void CreateWindowSurface(GLFWwindow& window);
		void CreateSwapChain(GLFWwindow& window);
		void CreateGraphicsPipeline();
		VkShaderModule CreateShaderModule(std::vector<char>& shaderCode);
		void CreateRenderPass();
		void CreateFrameBuffers();
		void CreateCommandPool();
		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t frameBufferIndex);
		void CreateSynchronizationObjects();
		void CreateVulkanMemoryAllocator();
		void CreateBuffer();
		VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags flags);
		VKFrameData& GetCurrentFrame();
		void TransitionImage(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
		VkImageSubresourceRange ImageSubresourceRange(VkImageAspectFlags aspectMask);
		VkCommandBufferSubmitInfo CommandBufferSubmitInfo(VkCommandBuffer cmdBuffer);
		VkSemaphoreSubmitInfo SemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);
		VkSubmitInfo2 SubmitInfo(VkCommandBufferSubmitInfo* cmdBufferInfo, VkSemaphoreSubmitInfo* signalSemaphore, VkSemaphoreSubmitInfo* waitSemaphore);

	private:
		VkInstance pInstance = VK_NULL_HANDLE;;
		std::vector<const char*> instanceExtensions = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
		std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
		VkDebugUtilsMessengerEXT pDebugUtilsMessanger = VK_NULL_HANDLE;;

		VkPhysicalDevice pPhysicalDevice = VK_NULL_HANDLE;
		VkDevice pDevice = VK_NULL_HANDLE;
		std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		VkSurfaceKHR pSurface = VK_NULL_HANDLE;

		unsigned int graphicsQueueIndex;
		VkQueue pGraphicsQueue = VK_NULL_HANDLE;
		VkQueue pPresentQueue = VK_NULL_HANDLE;

		VkSwapchainKHR pSwapChain = VK_NULL_HANDLE;
		std::vector<VkImage> swapChainImages;
		std::vector<VkImageView> swapChainImageViews;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;
		std::vector<VkFramebuffer> swapChainFramebuffers;

		VkRenderPass pRenderPass = VK_NULL_HANDLE;
		VkPipelineLayout pPipelineLayout = VK_NULL_HANDLE;
		VkPipeline pGraphicsPipeline = VK_NULL_HANDLE;

		VKAllocatedImage drawImage;
		VkExtent2D drawExtent;

		VKDeletionQueue mainDeletionQueue;

		VmaAllocator vmaAllocator = nullptr;

	public:
		VKFrameData frames[frameOverlap];
		uint32_t frameNumber = 0;

#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif
	};
}



