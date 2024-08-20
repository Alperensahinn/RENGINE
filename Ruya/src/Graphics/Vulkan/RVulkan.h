#pragma once

#include "RVulkanConfig.h"

#include <vector>
#include <deque>
#include <functional>

struct GLFWwindow;

namespace Ruya
{
	struct RVkDeletionQueue
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

	struct RVkFrameData
	{
		VkCommandPool commandPool = VK_NULL_HANDLE;
		VkCommandBuffer mainCommandBuffer = VK_NULL_HANDLE;

		VkSemaphore swapchainSemaphore = VK_NULL_HANDLE;;
		VkSemaphore renderSemaphore = VK_NULL_HANDLE;;
		VkFence renderFence = VK_NULL_HANDLE;;

		RVkDeletionQueue deletionQueue;
	};

	struct RVkAllocatedImage
	{
		VkImage image;
		VkImageView imageView;
		VmaAllocation allocation;
		VkExtent3D imageExtent;
		VkFormat imageFormat;
	};

	struct RVkDescriptorLayoutBuilder
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings;

		void AddBinding();
		void Clear();
	};

	constexpr uint32_t frameOverlap = 2;

	struct RVulkan
	{
#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif
	public:
		VkInstance	pInstance;
		VkPhysicalDevice pPhysicalDevice;
		VkDevice pDevice;
		VkSurfaceKHR pSurface;
		VkDebugUtilsMessengerEXT pDebugUtilsMessanger;
		uint32_t graphicsQueueIndex;
		VkQueue pGraphicsQueue;
		VkQueue pPresentQueue;
		VkSwapchainKHR pSwapChain;
		std::vector<VkImage> swapChainImages;
		std::vector<VkImageView> swapChainImageViews;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;
		std::vector<VkFramebuffer> swapChainFramebuffers;
		VkRenderPass pRenderPass;
		VkPipelineLayout pPipelineLayout;
		VkPipeline pGraphicsPipeline;
		RVkAllocatedImage drawImage;
		VkExtent2D drawExtent;
		RVkDeletionQueue mainDeletionQueue;
		VmaAllocator vmaAllocator;

		std::vector<const char*> instanceExtensions = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
		std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
		std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		RVkFrameData frames[frameOverlap];
		uint32_t frameNumber = 0;

	public:
		RVulkan(GLFWwindow& window);
		~RVulkan();

		RVulkan(const RVulkan&) = delete;
		RVulkan& operator=(const RVulkan&) = delete;

	public:
		void Init(GLFWwindow& window);
		void CleanUp();

		void Draw();
		RVkFrameData& GetCurrentFrame();
	};

	void rvkCreateInstance(RVulkan* pRVulkan);
	bool rvkCheckValidationLayerSupport(RVulkan* pRVulkan);
	void rvkCreateDebugMessenger(RVulkan* pRVulkan);
	void rvkDestroyDebugUtilsMessenger(RVulkan* pRVulkan);
	void rvkSelectPhysicalDevice(RVulkan* pRVulkan);
	void rvkCheckQueueFamilies(RVulkan* pRVulkan);
	void rvkCreateDevice(RVulkan* pRVulkan);
	void rvkSetQueues(RVulkan* pRVulkan);
	void rvkCreateWindowSurface(RVulkan* pRVulkan, GLFWwindow& window);
	void rvkCreateSwapChain(RVulkan* pRVulkan, GLFWwindow& window);
	void rvkCreateGraphicsPipeline(RVulkan* pRVulkan);
	VkShaderModule rvkCreateShaderModule(RVulkan* pRVulkan, std::vector<char>& shaderCode);
	void rvkCreateRenderPass(RVulkan* pRVulkan);
	void rvkCreateFrameBuffers(RVulkan* pRVulkan);
	void rvkCreateCommandPool(RVulkan* pRVulkan);
	void rvkRecordCommandBuffer(RVulkan* pRVulkan, VkCommandBuffer commandBuffer, uint32_t frameBufferIndex);
	void rvkCreateSynchronizationObjects(RVulkan* pRVulkan);
	void rvkCreateVulkanMemoryAllocator(RVulkan* pRVulkan);
	void rvkCreateBuffer();
	VkCommandBufferBeginInfo rvkCommandBufferBeginInfo(VkCommandBufferUsageFlags flags);
	void rvkTransitionImage(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
	VkImageSubresourceRange rvkImageSubresourceRange(VkImageAspectFlags aspectMask);
	VkCommandBufferSubmitInfo rvkCommandBufferSubmitInfo(VkCommandBuffer cmdBuffer);
	VkSemaphoreSubmitInfo rvkSemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);
	VkSubmitInfo2 rvkSubmitInfo(VkCommandBufferSubmitInfo* cmdBufferInfo, VkSemaphoreSubmitInfo* signalSemaphore, VkSemaphoreSubmitInfo* waitSemaphore);
	VkImageCreateInfo rvkImageCreateInfo(VkFormat format, VkImageUsageFlags imageUsageFlags, VkExtent3D extent);
	VkImageViewCreateInfo rvkImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);
	void rvkCopyImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize);
}



