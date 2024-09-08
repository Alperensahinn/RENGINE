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

	struct RVkRenderContext
	{
		VkDevice pDevice;
		VmaAllocator pVmaAllocator;
	};


	enum class MaterialPass : uint8_t 
	{
		MainColor,
		Transparent,
		Other
	};

	struct RVkMaterialPipeline
	{
		VkPipeline pipeline;
		VkPipelineLayout layout;
	};

	struct RVkMaterialInstance
	{
		RVkMaterialPipeline* pipeline;
		VkDescriptorSet materialSet;
		MaterialPass passType;
	};

	struct RVkMaterial 
	{
		RVkMaterialInstance data;
	};

	struct RVkRenderObject
	{
		uint32_t indexCount;
		uint32_t firstIndex;
		VkBuffer indexBuffer;

		std::shared_ptr<RVkMaterial> material;

		glm::mat4 transform;
		VkDeviceAddress vertexBufferAddress;
	};

	struct RVkGlobalUniformData
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
		RVkDescriptorAllocator descriptorAllocator;

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
		VkBuffer vkBuffer;
		VmaAllocation vmaAllocation;
		VmaAllocationInfo vmaAllocationInfo;

		void Destroy(RVulkan* pRvulkan);
	};

	struct RVkMeshBuffer
	{
		RVkAllocatedBuffer vertexBuffer;
		RVkAllocatedBuffer indexBuffer;
		VkDeviceAddress vertexBufferAddress;
		uint32_t indexCount;
	};

	struct RVkDrawPushConstants 
	{
		math::mat4 model;
		VkDeviceAddress vertexBuffer;
	};

	struct RVkSceneData
	{
		math::mat4 view;
		math::mat4 proj;
		math::mat4 viewproj;
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

	struct RVkMetallicRoughness
	{
		RVkMaterialPipeline opaquePipeline;

		VkDescriptorSetLayout materialLayout;

		struct MaterialResources 
		{
			RVkAllocatedImage albedoImage;
			VkSampler albedoSampler;
		};

		RVkDescriptorWriter writer;

		void BuildPipelines(RVulkan* pRVulkan);
		void ClearResources(RVulkan* pRVulkan);

		RVkMaterialInstance WriteMaterial(RVulkan* pRVulkan, MaterialPass pass, const MaterialResources& resources, RVkDescriptorAllocator& descriptorAllocator);
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
		
		uint32_t graphicsQueueIndex;
		uint32_t transferQueueIndex;
		uint32_t computeQueueIndex;

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
		RVkAllocatedImage depthImage;
		VkDescriptorSet drawImageDescriptors;
		VkDescriptorSetLayout drawImageDescriptorLayout;
		VkExtent2D drawExtent;
		uint32_t currentImageIndex;

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

		RVkGlobalUniformData globalUniformData;
		VkDescriptorSetLayout globalUniformDataDescriptorLayout;

		RVkMetallicRoughness metallicRoughnessPipeline;
		VkSampler defaultSamplerNearest;

	private:
		VkRenderingInfo renderInfo;

	public:
		RVulkan(GLFWwindow& window);
		~RVulkan();

		RVulkan(const RVulkan&) = delete;
		RVulkan& operator=(const RVulkan&) = delete;

	public:
		void Init(GLFWwindow& window);
		void WaitDeviceForCleanUp();
		void CleanUp();


		void BeginDraw();
		void Draw(RVkMeshBuffer meshBuffer, RVkMaterialInstance materialInstance, math::mat4 viewMatrix);
		void DrawEngineUI(EngineUI* pEngineUI);
		void EndDraw();
		RVkFrameData& GetCurrentFrame();

		void ResizeSwapChain();
	};

	void rvkCreateEngineUIDescriptorPool(RVulkan* pRVulkan);
	void rvkCreatePipelines(RVulkan* pRVulkan);


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
	VkRenderingInfo rvkCreateRenderingInfo(VkExtent2D renderExtent, VkRenderingAttachmentInfo* colorAttachment, VkRenderingAttachmentInfo* depthAttachment);

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

	void rvkWaitFences(RVulkan* pRVulkan, VkFence fence);
}



