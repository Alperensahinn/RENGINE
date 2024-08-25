#pragma once

#include "RVulkanConfig.h"
#include "../../Collections/RDeletionQueue.h"
#include "../../Utilities/Math/RMath.h"

#include <vector>
#include <span>

struct GLFWwindow;

namespace Ruya
{
	struct RVulkan;
	class EngineUI;

	struct RVkFrameData
	{
		VkCommandPool commandPool = VK_NULL_HANDLE;
		VkCommandBuffer mainCommandBuffer = VK_NULL_HANDLE;

		VkSemaphore swapchainSemaphore = VK_NULL_HANDLE;;
		VkSemaphore renderSemaphore = VK_NULL_HANDLE;;
		VkFence renderFence = VK_NULL_HANDLE;;

		RDeletionQueue deletionQueue;
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

		void AddBinding(uint32_t binding, VkDescriptorType descriptorType);
		void Clear();
		VkDescriptorSetLayout Build(RVulkan* pRVulkan, VkShaderStageFlags shaderStageFlags, void* pNext = nullptr, VkDescriptorSetLayoutCreateFlags dcsSetLayoutCreateflags = 0);
	};

	struct DescriptorAllocator
	{
		struct PoolSizeRatio
		{
			VkDescriptorType descriptorType;
			float ratio;
		};

		VkDescriptorPool descriptorPool;

		void InitPool(RVulkan* pRVulkan, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios);
		void ClearDescriptors(RVulkan* pRVulkan);
		void DestroyPool(RVulkan* pRVulkan);

		VkDescriptorSet Allocate(RVulkan* pRVulkan, VkDescriptorSetLayout layout);
	};

	struct ComputePushConstants 
	{
		math::vec4 data1;
		math::vec4 data2;
		math::vec4 data3;
		math::vec4 data4;
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

		VkPipeline pComputePipeline;
		VkPipelineLayout pComputePipelineLayout;

		RVkAllocatedImage drawImage;
		VkDescriptorSet drawImageDescriptors;
		VkDescriptorSetLayout drawImageDescriptorLayout;
		VkExtent2D drawExtent;

		VmaAllocator vmaAllocator;
		RDeletionQueue deletionQueue;
	
		DescriptorAllocator globalDescriptorAllocator;

		VkDescriptorPool immediateUIPool;

		std::vector<const char*> instanceExtensions = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME, "VK_KHR_win32_surface"};
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
		void WaitDeviceForCleanUp();
		void CleanUp();

		void Draw(EngineUI* pEngineUI);
		RVkFrameData& GetCurrentFrame();

	private:
		void DrawEngineUI(EngineUI* pEngineUI, VkCommandBuffer cmd, VkImageView targetImageView);
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
	void rvkCreatePipelines(RVulkan* pRVulkan);
	VkShaderModule rvkCreateShaderModule(RVulkan* pRVulkan, std::vector<char>& shaderCode);
	void rvkCreateRenderPass(RVulkan* pRVulkan);
	void rvkCreateFrameBuffers(RVulkan* pRVulkan);
	void rvkCreateCommandPool(RVulkan* pRVulkan);
	void rvkCreateSynchronizationObjects(RVulkan* pRVulkan);
	void rvkCreateVulkanMemoryAllocator(RVulkan* pRVulkan);
	void rvkCreateBuffer();
	void rvkCreateDescriptors(RVulkan* pRVulkan);
	VkCommandBufferBeginInfo rvkCommandBufferBeginInfo(VkCommandBufferUsageFlags flags);
	void rvkTransitionImage(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
	VkImageSubresourceRange rvkImageSubresourceRange(VkImageAspectFlags aspectMask);
	VkCommandBufferSubmitInfo rvkCommandBufferSubmitInfo(VkCommandBuffer cmdBuffer);
	VkSemaphoreSubmitInfo rvkSemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);
	VkSubmitInfo2 rvkSubmitInfo(VkCommandBufferSubmitInfo* cmdBufferInfo, VkSemaphoreSubmitInfo* signalSemaphore, VkSemaphoreSubmitInfo* waitSemaphore);
	VkImageCreateInfo rvkImageCreateInfo(VkFormat format, VkImageUsageFlags imageUsageFlags, VkExtent3D extent);
	VkImageViewCreateInfo rvkImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);
	void rvkCopyImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize);
	void rvkCreateEngineUIDescriptorPool(RVulkan* pRVulkan);
	VkRenderingAttachmentInfo rvkCreateAttachmentInfo(VkImageView view, VkClearValue* clear, VkImageLayout layout);
	VkRenderingInfo rvkCreateRenderingInfo(VkExtent2D renderExtent, VkRenderingAttachmentInfo* colorAttachment, VkRenderingAttachmentInfo* depthAttachment);
}



