#pragma once

#include "RVulkanConfig.h"
#include "../../Collections/RDeletionQueue.h"
#include "../../Utilities/Math/RMath.h"
#include "../Mesh.h"

#include <vector>
#include <span>

struct GLFWwindow;

namespace Ruya
{
	class RVulkan;
	class EngineUI;

	struct RVkMaterialPipeline
	{
		VkPipeline pipeline;
		VkPipelineLayout layout;
	};

	struct RVkMaterialInstance
	{
		RVkMaterialPipeline* pipeline;
		VkDescriptorSet materialSet;
		//MaterialPass passType;
	};

	struct RVkRenderObject
	{
		uint32_t indexCount;
		uint32_t firstIndex;
		VkBuffer indexBuffer;

		RVkMaterialInstance* material;

		glm::mat4 transform;
		VkDeviceAddress vertexBufferAddress;
	};

	struct GPUSceneData
	{
		math::mat4 view;
		math::mat4 proj;
		math::mat4 viewproj;
	};

	struct RVkDescriptorAllocator
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

	struct RVkFrameData
	{
		VkCommandPool commandPool = VK_NULL_HANDLE;
		VkCommandBuffer mainCommandBuffer = VK_NULL_HANDLE;

		VkSemaphore swapchainSemaphore = VK_NULL_HANDLE;;
		VkSemaphore renderSemaphore = VK_NULL_HANDLE;;
		VkFence renderFence = VK_NULL_HANDLE;;

		RDeletionQueue deletionQueue;
		RVkDescriptorAllocator descriptorAllocator;
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

	constexpr uint32_t frameOverlap = 2;

	class RVkPipelineBuilder
	{
	public:
		RVkPipelineBuilder();
		~RVkPipelineBuilder();

		RVkPipelineBuilder(const RVkPipelineBuilder&) = delete;
		RVkPipelineBuilder& operator=(const RVkPipelineBuilder&) = delete;

	public:
		VkPipeline BuildPipeline(RVulkan*  pRVulkan);
		void Clear();
		
		void SetShaders(VkShaderModule vertexShader, VkShaderModule fragmentShader);
		void SetInputTopology(VkPrimitiveTopology topology);
		void SetPolygonMode(VkPolygonMode polygonMode);
		void SetCullMode(VkCullModeFlags cullModeFlags, VkFrontFace frontFace);
		void SetMultisampling(bool b);
		void SetBlending(bool b);
		void SetColorAttachmentFormat(VkFormat format);
		void SetDepthFormat(VkFormat format);
		void SetDepthTest(bool b, VkCompareOp op);


	public:
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo;
		VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachmentState;
		VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo;
		VkPipelineLayout pipelineLayout;
		VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo;
		VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo;
		VkFormat colorAttachmentformat;
	};

	struct RVkAllocatedBuffer
	{
		VkBuffer buffer;
		VmaAllocation allocation;
		VmaAllocationInfo allocationInfo;
	};

	struct RVkMeshBuffer
	{
		RVkAllocatedBuffer vertexBuffer;
		RVkAllocatedBuffer indexBuffer;
		VkDeviceAddress vertexBufferAddress;
		uint32_t indexCount;
	};

	struct RVkDrawPushConstants {
		math::mat4 worldMatrix;
		VkDeviceAddress vertexBuffer;
	};

	struct RVkDescriptorWriter
	{
		std::deque<VkDescriptorImageInfo> imageInfos;
		std::deque<VkDescriptorBufferInfo> bufferInfos;
		std::vector<VkWriteDescriptorSet> writes;
		
		void WriteBuffer(uint32_t binding, VkDescriptorType descriptorType, VkBuffer buffer, size_t range, size_t offset);
		void WriteImage(uint32_t binding, VkDescriptorType descriptorType, VkImageView imageView, VkSampler sampler, VkImageLayout layout);
		void UpdateDescriptorSets(RVulkan* pRVulkan, VkDescriptorSet dstSet);
		void Clear();
	};

	class RVulkan
	{
	public:
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

		VkPipeline pGraphicsPipeline;

		VkPipelineLayout trianglePipelineLayout;
		VkPipeline trianglePipeline;

		RVkAllocatedImage drawImage;
		RVkAllocatedImage depthImage;
		VkDescriptorSet drawImageDescriptors;
		VkDescriptorSetLayout drawImageDescriptorLayout;
		VkExtent2D drawExtent;

		VmaAllocator vmaAllocator;
		RDeletionQueue deletionQueue;
	
		RVkDescriptorAllocator globalDescriptorAllocator;

		VkDescriptorPool immediateUIPool;

		VkFence immediateFence;
		VkCommandBuffer immediateCommandBuffer;
		VkCommandPool immediateCommandPool;

		std::vector<const char*> instanceExtensions = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME, "VK_KHR_win32_surface"};
		std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
		std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME};

		RVkFrameData frames[frameOverlap];
		uint32_t frameNumber = 0;

		GLFWwindow& window;

		bool resizeRequest = false;

		GPUSceneData sceneData;
		VkDescriptorSetLayout gpuSceneDataDescriptorSetLayout;

		//texture test
		RVkAllocatedImage test_texture;
		VkSampler defaultSamplerNearest;
		VkDescriptorSetLayout singleImageDescriptorLayout;


	public:
		RVulkan(GLFWwindow& window);
		~RVulkan();

		RVulkan(const RVulkan&) = delete;
		RVulkan& operator=(const RVulkan&) = delete;

	public:
		void Init(GLFWwindow& window);
		void WaitDeviceForCleanUp();
		void CleanUp();

		void Draw(EngineUI* pEngineUI, RVkMeshBuffer geometry, math::mat4 viewMatrix);
		RVkFrameData& GetCurrentFrame();

		void ResizeSwapChain();

	private:
		void DrawEngineUI(EngineUI* pEngineUI, VkCommandBuffer cmd, VkImageView targetImageView);
		void DrawGeometry(VkCommandBuffer cmdBuffer, RVkMeshBuffer geometry, math::mat4 viewMatrix);
		void CreateTrianglePipeline();
	};

	void rvkCreateInstance(RVulkan* pRVulkan);
	bool rvkCheckValidationLayerSupport(RVulkan* pRVulkan);
	void rvkCreateDebugMessenger(RVulkan* pRVulkan);
	void rvkDestroyDebugUtilsMessenger(RVulkan* pRVulkan);
	void rvkSelectPhysicalDevice(RVulkan* pRVulkan);
	void rvkCheckQueueFamilies(RVulkan* pRVulkan);
	void rvkCreateDevice(RVulkan* pRVulkan);
	void rvkSetQueues(RVulkan* pRVulkan);
	void rvkCreateWindowSurface(RVulkan* pRVulkan);
	void rvkCreateSwapChain(RVulkan* pRVulkan);
	VkShaderModule rvkCreateShaderModule(RVulkan* pRVulkan, std::vector<char>& shaderCode);
	void rvkCreateCommandPool(RVulkan* pRVulkan);
	void rvkCreateSynchronizationObjects(RVulkan* pRVulkan);
	void rvkCreateVulkanMemoryAllocator(RVulkan* pRVulkan);
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
	VkPipelineShaderStageCreateInfo rvkCreateShaderStageInfo(VkShaderModule shaderModule, VkShaderStageFlagBits shaderStageFlag);
	RVkAllocatedBuffer rvkCreateBuffer(RVulkan* pRulkan, size_t allocSize, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage);
	void rvkDestoryBuffer(RVulkan* pRulkan, RVkAllocatedBuffer& buffer);
	RVkMeshBuffer rvkLoadMesh(RVulkan* pRVulkan, std::vector<Vertex> vertices, std::vector<uint32_t> indices);
	void rvkImmediateSubmit(RVulkan* pRVulkan, std::function<void(VkCommandBuffer cmd)>&& function);
	VkRenderingAttachmentInfo  rvkDepthAttachmentInfo(VkImageView view, VkImageLayout layout);
	void rvkResizeSwapChain(RVulkan* pRVulkan);
	RVkAllocatedImage rvkCreateImage(RVulkan* pRVulkan, VkExtent3D extent, VkFormat format, VkImageUsageFlags usageFlags, bool mipmapped = false);
	RVkAllocatedImage rvkCreateImage(RVulkan* pRVulkan, void* data, VkExtent3D extent, VkFormat format, VkImageUsageFlags usageFlags, bool mipmapped = false);
	void rvkDestroyImage(RVulkan* pRVulkan, const RVkAllocatedImage& img);
}



