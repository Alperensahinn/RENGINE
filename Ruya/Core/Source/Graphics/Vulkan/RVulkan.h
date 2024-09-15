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
	struct PBRMaterial;


	struct alignas(16) RVkSceneData
	{
		math::mat4 view;
		math::mat4 proj;
		math::mat4 projView;
		math::vec3 viewPos;
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

		VkDescriptorSet Allocate(RVulkan* pRVulkan, VkDescriptorSetLayout layout, void* pNext);

	private:
		VkDescriptorPool GetPool(RVulkan* pRVulkan);
		VkDescriptorPool CreatePool(RVulkan* pRVulkan, uint32_t setCount, std::span<PoolSizeRatio> poolRatios);

		std::vector<PoolSizeRatio> ratios;
		std::vector<VkDescriptorPool> fullPools;
		std::vector<VkDescriptorPool> emptyPools;
		uint32_t setsPerPool;
	};

	struct RVkFrameData
	{
		VkCommandPool commandPool = VK_NULL_HANDLE;
		VkCommandBuffer mainCommandBuffer = VK_NULL_HANDLE;

		VkSemaphore swapchainSemaphore = VK_NULL_HANDLE;;
		VkSemaphore renderSemaphore = VK_NULL_HANDLE;;
		VkFence renderFence = VK_NULL_HANDLE;;

		RDeletionQueue deletionQueue;

		VkDescriptorPool descriptorPool;

		void ResetFrame(RVulkan* pRVulkan);
		uint32_t GetNextImage(RVulkan* pRVulkan);
		void BeginFrame(RVulkan* pRVulkan);
		void EndFrame(RVulkan* pRVulkan);
		void SubmitCommandBuffer(RVulkan* pRVulkan);
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
		VkPipeline BuildPipeline(RVulkan* pRVulkan, uint32_t passType);
		void Clear();
		
		void SetShaders(VkShaderModule vertexShader, VkShaderModule fragmentShader);
		void SetInputTopology(VkPrimitiveTopology topology);
		void SetPolygonMode(VkPolygonMode polygonMode);
		void SetCullMode(VkCullModeFlags cullModeFlags, VkFrontFace frontFace);
		void SetMultisampling(bool b);
		void SetBlending(bool b);
		void SetColorAttachmentFormat(VkFormat* colorAttachmentFormats, uint32_t colorAttachmentCount);
		void SetDepthFormat(VkFormat format);
		void SetDepthTest(bool b, VkCompareOp op);


	public:
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo;
		VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachmentState1;
		VkPipelineColorBlendAttachmentState colorBlendAttachmentState2;
		VkPipelineColorBlendAttachmentState colorBlendAttachmentState3;
		VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo;
		VkPipelineLayout pipelineLayout;
		VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo;
		VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo;
	};

	struct RVkAllocatedBuffer
	{
		VkBuffer vkBuffer;
		VmaAllocation vmaAllocation;
		VmaAllocationInfo vmaAllocationInfo;
	};

	struct RVkMeshBuffer
	{
		RVkAllocatedBuffer vertexBuffer;
		RVkAllocatedBuffer indexBuffer;
		VkDeviceAddress vertexBufferAddress;
		uint32_t indexCount;

		void Destroy(RVulkan* pRVulkan);
	};

	struct RVkDrawPushConstants 
	{
		math::mat4 model;
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

	struct RVkGBuffer
	{
		RVkAllocatedImage baseColorTexture;
		RVkAllocatedImage normalTexture;
		RVkAllocatedImage positionTexture;
		RVkAllocatedImage depthTexture;
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
		VkDebugUtilsMessengerEXT pDebugUtilsMessanger;
		VkPhysicalDevice pPhysicalDevice;
		VkDevice pDevice;
		VkSurfaceKHR pSurface;
		
		uint32_t graphicsQueueFamilyIndex;
		uint32_t transferQueueFamilyIndex;
		uint32_t computeQueueFamilyIndex;

		VkQueue pGraphicsQueue;
		VkQueue pTransferQueue;
		VkQueue pComputeQueue;
		VkQueue pPresentQueue;

		VkSwapchainKHR pSwapChain;
		std::vector<VkImage> swapChainImages;
		std::vector<VkImageView> swapChainImageViews;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;

		RVkAllocatedImage drawImage;
		VkExtent2D drawExtent;
		uint32_t currentImageIndex;
		RVkGBuffer gBuffer;

		VmaAllocator vmaAllocator;
		RDeletionQueue deletionQueue;

		VkDescriptorPool immediateUIPool;

		RVkDescriptorAllocator descriptorAllocator;

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

		RVkSceneData perframeSceneData;
		RVkAllocatedBuffer perframeSceneDataBuffer;
		VkDescriptorSetLayout perframeSceneDataDescriptorSetLayout;
		VkDescriptorSet perframeSceneDataDescriptorSet;
		RVkDescriptorWriter perframeSceneDataDescriptorWriter;

		VkPipeline pbrPipeline;
		VkPipelineLayout pbrPipelineLayout;
		VkDescriptorSetLayout pbrPipelineDescriptorSetLayoutMaterial;

		VkPipeline lightPassPipeline;
		VkPipelineLayout lightPassPipelineLayout;
		VkDescriptorSetLayout lightPassPipelineDescriptorSetLayout;
		VkDescriptorSet lightPassPipelineDescriptorSet;

		VkSampler defaultNearestSampler;
		VkSampler defaultLinearSampler;
	public:
		RVulkan(GLFWwindow& window);
		~RVulkan();

		RVulkan(const RVulkan&) = delete;
		RVulkan& operator=(const RVulkan&) = delete;

	public:
		void Init(GLFWwindow& window);
		void WaitDeviceForCleanUp();
		void CleanUp();


		void BeginFrame();
		void BeginDraw();
		void Draw(RVkMeshBuffer meshBuffer, PBRMaterial material, math::mat4 modelMatrix, math::mat4 viewMatrix, math::vec3 viewPos);
		void EndDraw();
		void LightPass();
		void DrawEngineUI(EngineUI* pEngineUI);
		void EndFrame();
		RVkFrameData& GetCurrentFrame();

		void ResizeSwapChain();
	};

	void rvkCreateEngineUIDescriptorPool(RVulkan* pRVulkan);

	//Init functions

	//Create vulkan instance
	bool rvkCheckValidationLayerSupport(RVulkan* pRVulkan);
	void rvkCreateInstance(RVulkan* pRVulkan);

	//Setup debug messenger
	void rvkCreateDebugMessenger(RVulkan* pRVulkan);
	void rvkDestroyDebugUtilsMessenger(RVulkan* pRVulkan);

	//Get phisycal device and avaiable ques;
	void rvkSelectPhysicalDevice(RVulkan* pRVulkan);

	void rvkCheckQueueFamilies(RVulkan* pRVulkan);

	//Create device and queues
	void rvkCreateDevice(RVulkan* pRVulkan);

	void rvkSetQueues(RVulkan* pRVulkan);

	//Create window surface for swap chain
	void rvkCreateWindowSurface(RVulkan* pRVulkan);

	//create swapchain
	void rvkCreateSwapChain(RVulkan* pRVulkan);

	//Create command pool and command buffers for all frames and immediate submit
	void rvkCreateCommandPool(RVulkan* pRVulkan);

	//Create sempahores and fences for all frames and immediate submit
	void rvkCreateSynchronizationObjects(RVulkan* pRVulkan);

	//Create Vulkan Memory Allocator for allocation resources (Vma libary instance)
	void rvkCreateVulkanMemoryAllocator(RVulkan* pRVulkan);

	//Create descriptor allocator
	void rvkCreateDescriptorAllocator(RVulkan* pRVulkan);

	//Creates PBR pipeline
	void rvkCreatePBRPipeline(RVulkan* pRVulkan);

	//Creates uniform buffer that hold per frame scene data
	void rvkCreatePerframeSceneUniformBuffer(RVulkan* pRVulkan);

	void rvkCreateGBuffer(RVulkan* pRVulkan);


	//Helper functions

	//Create shader module from bytecode
	VkShaderModule rvkCreateShaderModule(RVulkan* pRVulkan, std::vector<char>& shaderCode);

	//Creates and returns command buffer begin info for general usage
	VkCommandBufferBeginInfo rvkCreateCommandBufferBeginInfo(VkCommandBufferUsageFlags flags);

	//Creates and returns command buffer submit info for general usage
	VkCommandBufferSubmitInfo rvkCreateCommandBufferSubmitInfo(VkCommandBuffer cmdBuffer);
	
	//Creates and returns semaphore submit info for general usage
	VkSemaphoreSubmitInfo rvkCreateSemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);

	//Creates and returns queue submit info for general usage
	VkSubmitInfo2 rvkCreateQueueSubmitInfo(VkCommandBufferSubmitInfo* cmdBufferInfo, VkSemaphoreSubmitInfo* signalSemaphore, VkSemaphoreSubmitInfo* waitSemaphore);

	//Creates and returns rendering attachment info  for general usage
	VkRenderingAttachmentInfo rvkCreateRenderingAttachmentInfo(VkImageView view, VkClearValue* clear, VkImageLayout layout);

	//Creates and returns rendering attachment info  for depth buffer
	VkRenderingAttachmentInfo  rvkCreateDepthRenderingAttachmentInfo(VkImageView view, VkImageLayout layout);

	//Creates and returns rendering info  for general usage
	VkRenderingInfo rvkCreateRenderingInfo(VkExtent2D renderExtent, VkRenderingAttachmentInfo* colorAttachments, uint32_t colorAttachmentCount, VkRenderingAttachmentInfo* depthAttachment);

	//Changes image layout
	void rvkImageLayoutTransition(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);

	//Gets image subresource range
	VkImageSubresourceRange rvkGetImageSubresourceRange(VkImageAspectFlags aspectMask);

	//Creates and returns image create info for general usage
	VkImageCreateInfo rvkCreateImageCreateInfo(VkFormat format, VkImageUsageFlags imageUsageFlags, VkExtent3D extent);

	//Creates and returns image view info for general usage
	VkImageViewCreateInfo rvkCreateImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);

	//Copies one image to another in gpu memory
	void rvkCopyImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize);

	//Creates image on gpu side
	RVkAllocatedImage rvkCreateImage(RVulkan* pRVulkan, VkExtent3D extent, VkFormat format, VkImageUsageFlags usageFlags, bool mipmapped = false);
	RVkAllocatedImage rvkCreateImage(RVulkan* pRVulkan, void* data, VkExtent3D extent, VkFormat format, VkImageUsageFlags usageFlags, bool mipmapped = false);

	//Destroy image on gpu side
	void rvkDestroyImage(RVulkan* pRVulkan, const RVkAllocatedImage& img);

	VkSampler rvkCreateSamplerNearest(RVulkan* pRVulkan);

	VkSampler rvkCreateSamplerLinear(RVulkan* pRVulkan);

	void rvkDestroySampler(RVulkan* pRVulkan, VkSampler sampler);

	//Creates and returns pipeline shaderStage create info for general usage
	VkPipelineShaderStageCreateInfo rvkCreatePipelineShaderStageCreateInfo(VkShaderModule shaderModule, VkShaderStageFlagBits shaderStageFlag);

	//Creates buffer
	RVkAllocatedBuffer rvkCreateBuffer(RVulkan* pRulkan, size_t allocSize, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage);

	//Destroy buffer
	void rvkDestoryBuffer(RVulkan* pRulkan, RVkAllocatedBuffer& buffer);

	//Creates mesh buffer on gpu side
	RVkMeshBuffer rvkCreateMeshBuffer(RVulkan* pRVulkan, std::vector<Vertex> vertices, std::vector<uint32_t> indices);
	
	//Destroy mesh buffer on gpu side
	void DestroyMeshBuffer(RVulkan* pRVulkan, RVkMeshBuffer& meshBuffer);

	//Immediately submits data to gpu
	void rvkImmediateSubmit(RVulkan* pRVulkan, std::function<void(VkCommandBuffer cmd)>&& function);

	//Destroy old and create new swapchain 
	void rvkResizeSwapChain(RVulkan* pRVulkan);

	void rvkDestroySwapChain(RVulkan* pRVulkan);

	//Wait for fences
	void rvkWaitFences(RVulkan* pRVulkan, VkFence fence);
}



