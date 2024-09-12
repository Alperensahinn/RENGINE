#include "RVulkan.h"
#include "../../Utilities/FileSystem/FileSystem.h"
#include "../../Utilities/Log/RLog.h"
#include <Graphics/Renderer/Material.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <array>

namespace Ruya 
{
	RVulkan::RVulkan(GLFWwindow& window) : window(window)
	{
		Init(window);
	}

	RVulkan::~RVulkan()
	{
		CleanUp();
	}

	void RVulkan::Init(GLFWwindow& window)
	{
		rvkCreateInstance(this);
		rvkCreateDebugMessenger(this);
		rvkCreateWindowSurface(this);
		rvkSelectPhysicalDevice(this);
		rvkCheckQueueFamilies(this);
		rvkCreateDevice(this);
		rvkCreateVulkanMemoryAllocator(this);
		rvkCreateCommandPool(this);
		rvkCreateSynchronizationObjects(this);
		rvkCreateSwapChain(this);
		rvkCreateGBuffer(this);
		rvkSetQueues(this);
		rvkCreateEngineUIDescriptorPool(this);
		rvkCreateDescriptorAllocator(this);
		rvkCreatePBRPipeline(this);
		rvkCreateSceneUniformBuffer(this);
	}

	void RVulkan::WaitDeviceForCleanUp()
	{
		vkDeviceWaitIdle(pDevice);
	}

	void RVulkan::CleanUp()
	{
		deletionQueue.flush();
	}

	void RVulkan::BeginFrame()
	{
		RVkFrameData frameData = GetCurrentFrame();

		rvkWaitFences(this, frameData.renderFence);

		frameData.ResetFrame(this);

		currentImageIndex = frameData.GetNextImage(this);
		if (currentImageIndex == -1)
			return;

		frameData.BeginFrame(this);

		int glfwWindowWidth;
		int glfwWindowHeight;

		glfwGetFramebufferSize(&window, &glfwWindowWidth, &glfwWindowHeight);

		if (glfwWindowWidth == 0 || glfwWindowHeight == 0) {
			glfwWindowWidth = 1;
			glfwWindowHeight = 1;
		}

		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = static_cast<uint32_t>(glfwWindowWidth);
		viewport.height = static_cast<uint32_t>(glfwWindowHeight);
		viewport.minDepth = 1.f;
		viewport.maxDepth = 0.f;

		vkCmdSetViewport(frameData.mainCommandBuffer, 0, 1, &viewport);

		VkRect2D scissor = {};
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		scissor.extent.width = static_cast<uint32_t>(glfwWindowWidth);
		scissor.extent.height = static_cast<uint32_t>(glfwWindowHeight);

		vkCmdSetScissor(frameData.mainCommandBuffer, 0, 1, &scissor);
	}

	void RVulkan::BeginDraw()
	{
		VkCommandBuffer cmdBuffer = GetCurrentFrame().mainCommandBuffer;

		VkRenderingAttachmentInfo colorAttachment0 = rvkCreateRenderingAttachmentInfo(gBuffer.baseColorTexture.imageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		VkRenderingAttachmentInfo colorAttachment1 = rvkCreateRenderingAttachmentInfo(gBuffer.positionTexture.imageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		VkRenderingAttachmentInfo colorAttachment2 = rvkCreateRenderingAttachmentInfo(gBuffer.normalTexture.imageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		VkRenderingAttachmentInfo depthAttachment = rvkCreateDepthRenderingAttachmentInfo(gBuffer.depthTexture.imageView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

		VkRenderingAttachmentInfo colorAttachments[3]; 
		colorAttachments[0] = colorAttachment0;
		colorAttachments[1] = colorAttachment1;
		colorAttachments[2] = colorAttachment2;

		drawExtent.width = 1920;
		drawExtent.height = 1080;

		renderInfo = rvkCreateRenderingInfo(drawExtent, colorAttachments, 3, &depthAttachment);

		vkCmdBeginRendering(cmdBuffer, &renderInfo);
	}

	void RVulkan::Draw(RVkMeshBuffer meshBuffer, PBRMaterial material, math::mat4 viewMatrix)
	{	
		VkCommandBuffer cmdBuffer = GetCurrentFrame().mainCommandBuffer;

		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pbrPipeline);

		VkDescriptorSet descriptorSets[] = { perframeSceneDataDescriptorSet, material.descriptorSetMaterial };

		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pbrPipelineLayout, 0, 2, descriptorSets, 0, nullptr);

		glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)1600 / (float)900, 0.1f, 100.0f);
		glm::mat4 model = glm::mat4(1.0f);
		proj[1][1] *= -1;
		RVkDrawPushConstants push_constants;
		push_constants.model = proj * viewMatrix * model;
		push_constants.vertexBuffer = meshBuffer.vertexBufferAddress;
		vkCmdPushConstants(cmdBuffer, pbrPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(RVkDrawPushConstants), &push_constants);
		
		vkCmdBindIndexBuffer(cmdBuffer, meshBuffer.indexBuffer.vkBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmdBuffer, meshBuffer.indexCount, 1, 0, 0, 0);
	}

	void RVulkan::EndDraw()
	{
		VkCommandBuffer cmdBuffer = GetCurrentFrame().mainCommandBuffer;
		vkCmdEndRendering(cmdBuffer);
	}

	void RVulkan::EndFrame()
	{
		RVkFrameData frameData = GetCurrentFrame();
		frameData.EndFrame(this);
		frameData.SubmitCommandBuffer(this);

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.pSwapchains = &pSwapChain;
		presentInfo.swapchainCount = 1;

		presentInfo.pWaitSemaphores = &GetCurrentFrame().renderSemaphore;
		presentInfo.waitSemaphoreCount = 1;

		presentInfo.pImageIndices = &currentImageIndex;

		VkResult queuePresentResult = vkQueuePresentKHR(pGraphicsQueue, &presentInfo);

		if (queuePresentResult == VK_ERROR_OUT_OF_DATE_KHR)
		{
			resizeRequest = true;
		}

		frameNumber++;
	}

	RVkFrameData& RVulkan::GetCurrentFrame()
	{
		return frames[frameNumber % frameOverlap];
	}

	void RVulkan::ResizeSwapChain()
	{
		rvkResizeSwapChain(this);
		resizeRequest = false;
	}

	void RVulkan::DrawEngineUI(EngineUI* pEngineUI)
	{
		VkCommandBuffer cmdBuffer = GetCurrentFrame().mainCommandBuffer;

		VkRenderingAttachmentInfo colorAttachment = rvkCreateRenderingAttachmentInfo(swapChainImageViews[currentImageIndex], nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		VkRenderingInfo renderInfo = rvkCreateRenderingInfo(swapChainExtent, &colorAttachment, 1, nullptr);

		vkCmdBeginRendering(cmdBuffer, &renderInfo);

		pEngineUI->DrawData(cmdBuffer);

		vkCmdEndRendering(cmdBuffer);
	}

	void rvkCreateInstance(RVulkan* pRVulkan)
	{
		VkApplicationInfo applicationInfo = {};
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.apiVersion = VK_API_VERSION_1_3;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &applicationInfo;

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};


		if (pRVulkan->enableValidationLayers && !rvkCheckValidationLayerSupport(pRVulkan))
		{
			RERRLOG("[VULKAN ERROR] Validation layers requested, but not available.")
		}

		if (pRVulkan->enableValidationLayers && rvkCheckValidationLayerSupport(pRVulkan))
		{
			debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			debugCreateInfo.pfnUserCallback = debugCallback;

			createInfo.enabledLayerCount = static_cast<uint32_t>(pRVulkan->validationLayers.size());
			createInfo.ppEnabledLayerNames = pRVulkan->validationLayers.data();

			createInfo.enabledExtensionCount = static_cast<uint32_t>(pRVulkan->instanceExtensions.size());
			createInfo.ppEnabledExtensionNames = pRVulkan->instanceExtensions.data();

			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

			RLOG("[VULKAN] Validation layers enabled.");
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		CHECK_VKRESULT(vkCreateInstance(&createInfo, nullptr, &(pRVulkan->pInstance)));

		pRVulkan->deletionQueue.PushFunction([=]()
			{
				vkDestroyInstance(pRVulkan->pInstance, nullptr);
			});

		RLOG("[VULKAN] Vulkan instance created.")
	}

	bool rvkCheckValidationLayerSupport(RVulkan* pRVulkan)
	{
		uint32_t layerCount;
		CHECK_VKRESULT(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));
		std::vector<VkLayerProperties> availableLayers(layerCount);
		CHECK_VKRESULT(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()));

		for (const char* layerName : pRVulkan->validationLayers)
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}


	void rvkCreateDebugMessenger(RVulkan* pRVulkan)
	{
		if (!pRVulkan->enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback = debugCallback;

		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(pRVulkan->pInstance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			if (func((pRVulkan->pInstance), &debugCreateInfo, nullptr, &(pRVulkan->pDebugUtilsMessanger)) != VK_SUCCESS)
			{
				RERRLOG("[VULKAN ERROR] Failed to set up debug messenger.")
			}
		}
		else
		{
			RERRLOG("[VULKAN ERROR] Failed to find vkCreateDebugUtilsMessengerEXT.")
		}

		if (pRVulkan->enableValidationLayers)
		{
			pRVulkan->deletionQueue.PushFunction([=]()
				{
					rvkDestroyDebugUtilsMessenger(pRVulkan);
				});
		}

	}

	void rvkDestroyDebugUtilsMessenger(RVulkan* pRVulkan)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(pRVulkan->pInstance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(pRVulkan->pInstance, pRVulkan->pDebugUtilsMessanger, nullptr);
		}
	}

	void rvkSelectPhysicalDevice(RVulkan* pRVulkan)
	{
		unsigned int physicalDevicesCount;
		CHECK_VKRESULT(vkEnumeratePhysicalDevices(pRVulkan->pInstance, &physicalDevicesCount, nullptr));

		if (physicalDevicesCount == 0)
		{
			RERRLOG("[VULKAN ERROR] Failed to find Vulkan physical device.")
		}

		std::vector<VkPhysicalDevice> physicalDevices(physicalDevicesCount);
		CHECK_VKRESULT(vkEnumeratePhysicalDevices(pRVulkan->pInstance, &physicalDevicesCount, physicalDevices.data()));

		pRVulkan->pPhysicalDevice = physicalDevices[0];

		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(pRVulkan->pPhysicalDevice, &physicalDeviceProperties);
		RLOG("[VULKAN] Vulkan physical device selected: " << physicalDeviceProperties.deviceName)
	}

	void rvkCheckQueueFamilies(RVulkan* pRVulkan)
	{
		unsigned int queueFamilyPropertyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(pRVulkan->pPhysicalDevice, &queueFamilyPropertyCount, nullptr);

		if (queueFamilyPropertyCount == 0)
		{
			RERRLOG("[VULKAN ERROR] No queue families supported.");
		}

		std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(pRVulkan->pPhysicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());

		for (uint32_t i = 0; i < queueFamilyPropertyCount; i++)
		{
			const VkQueueFamilyProperties& queueFamily = queueFamilyProperties.at(i);

			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				pRVulkan->graphicsQueueFamilyIndex = i;
				RLOG("[VULKAN] Queue family with graphics bit found. Index: " << pRVulkan->graphicsQueueFamilyIndex);
			}

			if(queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				pRVulkan->transferQueueFamilyIndex = i;
				RLOG("[VULKAN] Queue family with transfer bit found. Index: " << pRVulkan->transferQueueFamilyIndex);
			}

			if(queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				pRVulkan->computeQueueFamilyIndex = i;
				RLOG("[VULKAN] Queue family with compute bit found. Index: " << pRVulkan->computeQueueFamilyIndex);
			}
		}
	}

	void rvkCreateDevice(RVulkan* pRVulkan)
	{
		float queuePriorities = 1.0f;
		VkDeviceQueueCreateInfo queueCreateInfo1 = {};
		queueCreateInfo1.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo1.queueFamilyIndex = pRVulkan->graphicsQueueFamilyIndex;
		queueCreateInfo1.queueCount = 1;
		queueCreateInfo1.pQueuePriorities = &queuePriorities;

		VkDeviceQueueCreateInfo queueCreateInfo2 = {};
		queueCreateInfo2.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo2.queueFamilyIndex = pRVulkan->transferQueueFamilyIndex;
		queueCreateInfo2.queueCount = 1;
		queueCreateInfo2.pQueuePriorities = &queuePriorities;

		VkDeviceQueueCreateInfo queueCreateInfo3 = {};
		queueCreateInfo3.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo3.queueFamilyIndex = pRVulkan->computeQueueFamilyIndex;
		queueCreateInfo3.queueCount = 1;
		queueCreateInfo3.pQueuePriorities = &queuePriorities;

		VkDeviceQueueCreateInfo queueCreateInfos[] = { queueCreateInfo1 , queueCreateInfo2 , queueCreateInfo3 };

		VkPhysicalDeviceVulkan13Features features13 = {};
		features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		features13.dynamicRendering = VK_TRUE;
		features13.synchronization2 = VK_TRUE;

		VkPhysicalDeviceBufferDeviceAddressFeatures physicalDeviceBufferDeviceAddressFeatures = {};
		physicalDeviceBufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
		physicalDeviceBufferDeviceAddressFeatures.pNext = &features13;
		physicalDeviceBufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;

		VkPhysicalDeviceFeatures2 features2 = {};
		features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features2.pNext = &physicalDeviceBufferDeviceAddressFeatures;

		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pNext = &features2;
		deviceCreateInfo.queueCreateInfoCount = 3;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
		deviceCreateInfo.pEnabledFeatures = nullptr;
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(pRVulkan->deviceExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = pRVulkan->deviceExtensions.data();

		CHECK_VKRESULT(vkCreateDevice(pRVulkan->pPhysicalDevice, &deviceCreateInfo, nullptr, &(pRVulkan->pDevice)));

		pRVulkan->deletionQueue.PushFunction([=]()
			{
				vkDestroyDevice(pRVulkan->pDevice, nullptr);
			});

		RLOG("[VULKAN] Logical device created.");
	}

	void rvkSetQueues(RVulkan* pRVulkan)
	{
		vkGetDeviceQueue(pRVulkan->pDevice, pRVulkan->graphicsQueueFamilyIndex, 0, &(pRVulkan->pGraphicsQueue));
		vkGetDeviceQueue(pRVulkan->pDevice, pRVulkan->computeQueueFamilyIndex, 0, &(pRVulkan->pTransferQueue));
		vkGetDeviceQueue(pRVulkan->pDevice, pRVulkan->transferQueueFamilyIndex, 0, &(pRVulkan->pComputeQueue));
		vkGetDeviceQueue(pRVulkan->pDevice, pRVulkan->graphicsQueueFamilyIndex, 0, &(pRVulkan->pPresentQueue));

		RLOG("[VULKAN] Device queues setted.");
	}

	void rvkCreateWindowSurface(RVulkan* pRVulkan)
	{
		VkSurfaceKHR surface;
		if (glfwCreateWindowSurface(pRVulkan->pInstance, &pRVulkan->window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface.");
		}
		
		pRVulkan->pSurface = surface;

		pRVulkan->deletionQueue.PushFunction([=]()
			{
				vkDestroySurfaceKHR(pRVulkan->pInstance, pRVulkan->pSurface, nullptr);
			});

		RLOG("[VULKAN] Window surface created.")
	}

	void rvkCreateSwapChain(RVulkan* pRVulkan)
	{
		VkSurfaceCapabilitiesKHR capabilities;
		CHECK_VKRESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pRVulkan->pPhysicalDevice, pRVulkan->pSurface, &capabilities))

		std::vector<VkSurfaceFormatKHR> surfaceFormats;
		uint32_t surfaceFormatCount;

		CHECK_VKRESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(pRVulkan->pPhysicalDevice, pRVulkan->pSurface, &surfaceFormatCount, nullptr))

			if (surfaceFormatCount > 0)
			{
				surfaceFormats.resize(surfaceFormatCount);
				CHECK_VKRESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(pRVulkan->pPhysicalDevice, pRVulkan->pSurface, &surfaceFormatCount, surfaceFormats.data()))
			}

		std::vector<VkPresentModeKHR> presentModes;
		uint32_t presentModeCount;

		CHECK_VKRESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(pRVulkan->pPhysicalDevice, pRVulkan->pSurface, &presentModeCount, nullptr))

			if (presentModeCount > 0)
			{
				presentModes.resize(presentModeCount);
				CHECK_VKRESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(pRVulkan->pPhysicalDevice, pRVulkan->pSurface, &presentModeCount, presentModes.data()))
			}

		if (presentModes.empty() || surfaceFormats.empty())
		{
			RERRLOG("[VULKAN ERROR] Device not support swap chain.");
			return;
		}


		VkSurfaceFormatKHR surfaceFormat;
		bool bSRGBAvailable = false;

		for (VkSurfaceFormatKHR format : surfaceFormats)
		{
			if (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && format.format == VK_FORMAT_B8G8R8A8_SRGB)
			{
				surfaceFormat = format;
				bSRGBAvailable = true;
			}
		}

		if (bSRGBAvailable == false)
		{
			surfaceFormat = surfaceFormats[0];
		}

		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;

		int width, height;
		glfwGetFramebufferSize(&(pRVulkan->window), &width, &height);

		VkExtent2D actualExtent =
		{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		uint32_t imageCount = capabilities.minImageCount + 1;

		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
		{
			imageCount = capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
		swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapChainCreateInfo.surface = pRVulkan->pSurface;
		swapChainCreateInfo.minImageCount = imageCount;
		swapChainCreateInfo.imageFormat = surfaceFormat.format;
		swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
		swapChainCreateInfo.imageExtent = actualExtent;
		swapChainCreateInfo.imageArrayLayers = 1;
		swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		uint32_t queueFamilyIndices[] = { 0,0 };

		if (pRVulkan->pGraphicsQueue == pRVulkan->pPresentQueue)
		{
			swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapChainCreateInfo.queueFamilyIndexCount = 0;
			swapChainCreateInfo.pQueueFamilyIndices = nullptr;
		}
		else
		{
			swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapChainCreateInfo.queueFamilyIndexCount = 2;
			swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
		}

		swapChainCreateInfo.preTransform = capabilities.currentTransform;
		swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapChainCreateInfo.presentMode = presentMode;
		swapChainCreateInfo.clipped = VK_TRUE;
		swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

		CHECK_VKRESULT(vkCreateSwapchainKHR(pRVulkan->pDevice, &swapChainCreateInfo, nullptr, &(pRVulkan->pSwapChain)));

		pRVulkan->deletionQueue.PushFunction([=]()
			{
				vkDestroySwapchainKHR(pRVulkan->pDevice, pRVulkan->pSwapChain, nullptr);
			});


		uint32_t swapChainImageCount;
		CHECK_VKRESULT(vkGetSwapchainImagesKHR(pRVulkan->pDevice, pRVulkan->pSwapChain, &swapChainImageCount, nullptr));
		pRVulkan->swapChainImages.resize(swapChainImageCount);
		pRVulkan->swapChainImageViews.resize(swapChainImageCount);
		CHECK_VKRESULT(vkGetSwapchainImagesKHR(pRVulkan->pDevice, pRVulkan->pSwapChain, &swapChainImageCount, pRVulkan->swapChainImages.data()));

		pRVulkan->swapChainImageFormat = surfaceFormat.format;
		pRVulkan->swapChainExtent = actualExtent;

		for (int i = 0; i < pRVulkan->swapChainImages.size(); i++)
		{
			VkImageViewCreateInfo imageViewCreateInfo = {};
			imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewCreateInfo.image = pRVulkan->swapChainImages.data()[i];
			imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewCreateInfo.format = pRVulkan->swapChainImageFormat;
			imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
			imageViewCreateInfo.subresourceRange.levelCount = 1;
			imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
			imageViewCreateInfo.subresourceRange.layerCount = 1;

			CHECK_VKRESULT(vkCreateImageView(pRVulkan->pDevice, &imageViewCreateInfo, nullptr, &(pRVulkan->swapChainImageViews.data()[i])));

			pRVulkan->deletionQueue.PushFunction([=]()
				{
					vkDestroyImageView(pRVulkan->pDevice, pRVulkan->swapChainImageViews[i], nullptr);
				});
		}


		VkExtent3D drawImageExtent = {
			1920,
			1080,
			1
		};

		pRVulkan->drawImage.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
		pRVulkan->drawImage.imageExtent = drawImageExtent;

		VkImageUsageFlags drawImageUsageFlags = {};
		drawImageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		drawImageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		drawImageUsageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
		drawImageUsageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		drawImageUsageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;

		VkImageCreateInfo renderImageCreateInfo = rvkCreateImageCreateInfo(pRVulkan->drawImage.imageFormat, drawImageUsageFlags, pRVulkan->drawImage.imageExtent);

		VmaAllocationCreateInfo rimgAllocCreateInfo = {};
		rimgAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		rimgAllocCreateInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		CHECK_VKRESULT(vmaCreateImage(pRVulkan->vmaAllocator, &renderImageCreateInfo, &rimgAllocCreateInfo, &(pRVulkan->drawImage.image), &(pRVulkan->drawImage.allocation), nullptr));

		VkImageViewCreateInfo rimgCreateInfo = rvkCreateImageViewCreateInfo(pRVulkan->drawImage.imageFormat, pRVulkan->drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

		CHECK_VKRESULT(vkCreateImageView(pRVulkan->pDevice, &rimgCreateInfo, nullptr, &(pRVulkan->drawImage.imageView)));

		pRVulkan->depthImage.imageFormat = VK_FORMAT_D32_SFLOAT;
		pRVulkan->depthImage.imageExtent = drawImageExtent;
		VkImageUsageFlags depthImageUsages = {};
		depthImageUsages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		VkImageCreateInfo dimg_info = rvkCreateImageCreateInfo(pRVulkan->depthImage.imageFormat, depthImageUsages, drawImageExtent);

		CHECK_VKRESULT(vmaCreateImage(pRVulkan->vmaAllocator, &dimg_info, &rimgAllocCreateInfo, &(pRVulkan->depthImage.image), &(pRVulkan->depthImage.allocation), nullptr));

		VkImageViewCreateInfo dview_info = rvkCreateImageViewCreateInfo(pRVulkan->depthImage.imageFormat, pRVulkan->depthImage.image, VK_IMAGE_ASPECT_DEPTH_BIT);

		CHECK_VKRESULT(vkCreateImageView(pRVulkan->pDevice, &dview_info, nullptr, &(pRVulkan->depthImage.imageView)));

		pRVulkan->deletionQueue.PushFunction([=]()
			{
				vmaDestroyImage(pRVulkan->vmaAllocator, pRVulkan->drawImage.image, pRVulkan->drawImage.allocation);
				vkDestroyImageView(pRVulkan->pDevice, pRVulkan->drawImage.imageView, nullptr);

				vmaDestroyImage(pRVulkan->vmaAllocator, pRVulkan->depthImage.image, pRVulkan->depthImage.allocation);
				vkDestroyImageView(pRVulkan->pDevice, pRVulkan->depthImage.imageView, nullptr);
			});

		RLOG("[VULKAN] Swap chain created.");
	}

	VkShaderModule rvkCreateShaderModule(RVulkan* pRVulkan, std::vector<char>& shaderCode)
	{
		VkShaderModule shaderModule;

		VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
		shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.codeSize = shaderCode.size();
		shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

		CHECK_VKRESULT(vkCreateShaderModule(pRVulkan->pDevice, &shaderModuleCreateInfo, nullptr, &shaderModule));

		RLOG("[VULKAN] Shader module created.")

		return shaderModule;
	}

	void rvkCreateCommandPool(RVulkan* pRVulkan)
	{
		VkCommandPoolCreateInfo commandPoolCreateInfo = {};
		commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolCreateInfo.queueFamilyIndex = pRVulkan->graphicsQueueFamilyIndex;

		for (int i = 0; i < frameOverlap; i++)
		{
			CHECK_VKRESULT(vkCreateCommandPool(pRVulkan->pDevice, &commandPoolCreateInfo, nullptr, &(pRVulkan->frames[i].commandPool)));

			VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
			commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			commandBufferAllocateInfo.commandPool = pRVulkan->frames[i].commandPool;
			commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			commandBufferAllocateInfo.commandBufferCount = 1;

			CHECK_VKRESULT(vkAllocateCommandBuffers(pRVulkan->pDevice, &commandBufferAllocateInfo, &(pRVulkan->frames[i].mainCommandBuffer)));

			pRVulkan->deletionQueue.PushFunction([=]()
				{
					vkDestroyCommandPool(pRVulkan->pDevice, pRVulkan->frames[i].commandPool, nullptr);
				});
		}
		
		CHECK_VKRESULT(vkCreateCommandPool(pRVulkan->pDevice, &commandPoolCreateInfo, nullptr, &(pRVulkan->immediateCommandPool)));

		VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.commandPool = pRVulkan->immediateCommandPool;
		commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandBufferCount = 1;

		CHECK_VKRESULT(vkAllocateCommandBuffers(pRVulkan->pDevice, &commandBufferAllocateInfo, &(pRVulkan->immediateCommandBuffer)));

		pRVulkan->deletionQueue.PushFunction([=]()
			{
				vkDestroyCommandPool(pRVulkan->pDevice, pRVulkan->immediateCommandPool, nullptr);
			});


		RLOG("[VULKAN] Command pools and buffers created.")
	}

	void rvkCreateSynchronizationObjects(RVulkan* pRVulkan)
	{
		for (int i = 0; i < frameOverlap; i++)
		{
			VkSemaphoreCreateInfo semaphoreCreateInfo = {};
			semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			CHECK_VKRESULT(vkCreateSemaphore(pRVulkan->pDevice, &semaphoreCreateInfo, nullptr, &(pRVulkan->frames[i].swapchainSemaphore)));
			CHECK_VKRESULT(vkCreateSemaphore(pRVulkan->pDevice, &semaphoreCreateInfo, nullptr, &(pRVulkan->frames[i].renderSemaphore)));

			VkFenceCreateInfo fenceCreateInfo = {};
			fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			CHECK_VKRESULT(vkCreateFence(pRVulkan->pDevice, &fenceCreateInfo, nullptr, &(pRVulkan->frames[i].renderFence)));

			pRVulkan->deletionQueue.PushFunction([=]()
				{
					vkDestroySemaphore(pRVulkan->pDevice, pRVulkan->frames[i].swapchainSemaphore, nullptr);
					vkDestroySemaphore(pRVulkan->pDevice, pRVulkan->frames[i].renderSemaphore, nullptr);
					vkDestroyFence(pRVulkan->pDevice, pRVulkan->frames[i].renderFence, nullptr);
				});
		}

		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		CHECK_VKRESULT(vkCreateFence(pRVulkan->pDevice, &fenceCreateInfo, nullptr, &(pRVulkan->immediateFence)));

		pRVulkan->deletionQueue.PushFunction([=]()
			{
				vkDestroyFence(pRVulkan->pDevice, pRVulkan->immediateFence, nullptr);

			});

		RLOG("[VULKAN] Synchronization objects created.")
	}

	void rvkCreateVulkanMemoryAllocator(RVulkan* pRVulkan)
	{
		VmaVulkanFunctions vulkanFunctions = {};
		vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
		vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

#if VMA_VULKAN_VERSION >= 1003000
		vulkanFunctions.vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements;
		vulkanFunctions.vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements;
#endif

		VmaAllocatorCreateInfo allocatorCreateInfo = {};
		allocatorCreateInfo.device = pRVulkan->pDevice;
		allocatorCreateInfo.instance = pRVulkan->pInstance;
		allocatorCreateInfo.physicalDevice = pRVulkan->pPhysicalDevice;
		allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
		allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
		allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

		CHECK_VKRESULT(vmaCreateAllocator(&allocatorCreateInfo, &(pRVulkan->vmaAllocator)));

		pRVulkan->deletionQueue.PushFunction([=]()
			{
				vmaDestroyAllocator(pRVulkan->vmaAllocator);
			});

		RLOG("[VULKAN] Vulkan memory allocator created.")
	}

	void rvkCreateDescriptorAllocator(RVulkan* pRVulkan)
	{
		std::vector<RVkDescriptorAllocator::PoolSizeRatio> ratios = 
		{
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 },
		};

		pRVulkan->descriptorAllocator.InitPool(pRVulkan, 100, ratios);

		pRVulkan->deletionQueue.PushFunction([=]()
			{
				pRVulkan->descriptorAllocator.DestroyPool(pRVulkan);
			});
	}

	void rvkCreatePBRPipeline(RVulkan* pRVulkan)
	{
		VkShaderModule vertexShader;
		std::vector<char> colorTriangleVertexCode = Ruya::ReadBinaryFile("C:\\Users\\aalpe\\Desktop\\RENGINE\\Ruya\\Core\\Source\\Graphics\\Shaders\\GBufferVertexShader.spv");
		vertexShader = rvkCreateShaderModule(pRVulkan, colorTriangleVertexCode);

		VkShaderModule fragmentShader;
		std::vector<char> colorTriangleFragmentCode = Ruya::ReadBinaryFile("C:\\Users\\aalpe\\Desktop\\RENGINE\\Ruya\\Core\\Source\\Graphics\\Shaders\\GBufferFragmentShader.spv");
		fragmentShader = rvkCreateShaderModule(pRVulkan, colorTriangleFragmentCode);

		VkPushConstantRange pushConstantRange = {};
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(RVkDrawPushConstants);
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		RVkDescriptorLayoutBuilder descriptorLayoutBuilder1;
		descriptorLayoutBuilder1.Clear();
		descriptorLayoutBuilder1.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		VkDescriptorSetLayout descriptorSetLayout1 = descriptorLayoutBuilder1.Build(pRVulkan, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

		pRVulkan->perframeSceneDataDescriptorSetLayout = descriptorSetLayout1;

		RVkDescriptorLayoutBuilder descriptorLayoutBuilder2;
		descriptorLayoutBuilder2.Clear();
		descriptorLayoutBuilder2.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		VkDescriptorSetLayout descriptorSetLayout2 = descriptorLayoutBuilder2.Build(pRVulkan, VK_SHADER_STAGE_FRAGMENT_BIT);

		pRVulkan->pbrPipelineDescriptorSetLayoutMaterial = descriptorSetLayout2;

		VkDescriptorSetLayout setLayouts[] = { descriptorSetLayout1 , descriptorSetLayout2 };

		VkPipelineLayoutCreateInfo pipeLineLayoutCreateInfo = {};
		pipeLineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeLineLayoutCreateInfo.setLayoutCount = 2;
		pipeLineLayoutCreateInfo.pSetLayouts = setLayouts;
		pipeLineLayoutCreateInfo.pushConstantRangeCount = 1;
		pipeLineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

		VkPipelineLayout pipelineLayout;
		CHECK_VKRESULT(vkCreatePipelineLayout(pRVulkan->pDevice, &pipeLineLayoutCreateInfo, nullptr, &pipelineLayout));

		pRVulkan->pbrPipelineLayout = pipelineLayout;

		RVkPipelineBuilder pipelineBuilder;
		pipelineBuilder.Clear();
		pipelineBuilder.SetShaders(vertexShader, fragmentShader);
		pipelineBuilder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		pipelineBuilder.SetPolygonMode(VK_POLYGON_MODE_FILL);
		pipelineBuilder.SetCullMode(VK_CULL_MODE_FRONT_BIT, VK_FRONT_FACE_CLOCKWISE);
		pipelineBuilder.SetMultisampling(false);
		pipelineBuilder.SetBlending(false);
		pipelineBuilder.SetDepthTest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);

		VkFormat colorAttachmentFortmats[] = { VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R16G16B16A16_SFLOAT };

		pipelineBuilder.SetColorAttachmentFormat(colorAttachmentFortmats, 3);
		pipelineBuilder.SetDepthFormat(pRVulkan->depthImage.imageFormat);
		pipelineBuilder.pipelineLayout = pipelineLayout;

		pRVulkan->pbrPipeline = pipelineBuilder.BuildPipeline(pRVulkan);

		vkDestroyShaderModule(pRVulkan->pDevice, vertexShader, nullptr);
		vkDestroyShaderModule(pRVulkan->pDevice, fragmentShader, nullptr);

		pRVulkan->defaultSampler = Ruya::rvkCreateSamplerNearest(pRVulkan);

		pRVulkan->deletionQueue.PushFunction([=]()
			{
				vkDestroyDescriptorSetLayout(pRVulkan->pDevice, pRVulkan->perframeSceneDataDescriptorSetLayout, nullptr);
				vkDestroyDescriptorSetLayout(pRVulkan->pDevice, pRVulkan->pbrPipelineDescriptorSetLayoutMaterial, nullptr);
				vkDestroyPipelineLayout(pRVulkan->pDevice, pRVulkan->pbrPipelineLayout, nullptr);
				vkDestroyPipeline(pRVulkan->pDevice, pRVulkan->pbrPipeline, nullptr);
				rvkDestroySampler(pRVulkan, pRVulkan->defaultSampler);
			});
	}

	void rvkCreateSceneUniformBuffer(RVulkan* pRVulkan)
	{
		pRVulkan->perframeSceneDataDescriptorSet = pRVulkan->descriptorAllocator.Allocate(pRVulkan, pRVulkan->perframeSceneDataDescriptorSetLayout, nullptr);

		pRVulkan->perframeSceneDataBuffer = Ruya::rvkCreateBuffer(pRVulkan, sizeof(RVkSceneData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		RVkSceneData bufferData;
		bufferData.view = math::mat4(1.0f);
		bufferData.proj = math::mat4(1.0f);
		bufferData.viewproj = math::mat4(1.0f);

		void* data;
		vmaMapMemory(pRVulkan->vmaAllocator, pRVulkan->perframeSceneDataBuffer.vmaAllocation, &data);
		memcpy(data, &bufferData, sizeof(RVkSceneData));
		vmaUnmapMemory(pRVulkan->vmaAllocator, pRVulkan->perframeSceneDataBuffer.vmaAllocation);

		pRVulkan->perframeSceneDataDescriptorWriter.WriteBuffer(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, pRVulkan->perframeSceneDataBuffer.vkBuffer, sizeof(RVkSceneData), 0);
		pRVulkan->perframeSceneDataDescriptorWriter.UpdateDescriptorSets(pRVulkan, pRVulkan->perframeSceneDataDescriptorSet);

		pRVulkan->deletionQueue.PushFunction([=]()
			{
				rvkDestoryBuffer(pRVulkan, pRVulkan->perframeSceneDataBuffer);
			});
	}

	void rvkCreateGBuffer(RVulkan* pRVulkan)
	{
		VkImageUsageFlags imageUsageFlags = {};
		imageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		imageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageUsageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
		imageUsageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		imageUsageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;

		VkImageCreateInfo imageCreateInfo = rvkCreateImageCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT, imageUsageFlags, pRVulkan->drawImage.imageExtent);

		VmaAllocationCreateInfo allocationCreateInfo = {};
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		allocationCreateInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		CHECK_VKRESULT(vmaCreateImage(pRVulkan->vmaAllocator, &imageCreateInfo, &allocationCreateInfo, &(pRVulkan->gBuffer.baseColorTexture.image), &(pRVulkan->gBuffer.baseColorTexture.allocation), nullptr));
		CHECK_VKRESULT(vmaCreateImage(pRVulkan->vmaAllocator, &imageCreateInfo, &allocationCreateInfo, &(pRVulkan->gBuffer.positionTexture.image), &(pRVulkan->gBuffer.positionTexture.allocation), nullptr));
		CHECK_VKRESULT(vmaCreateImage(pRVulkan->vmaAllocator, &imageCreateInfo, &allocationCreateInfo, &(pRVulkan->gBuffer.normalTexture.image), &(pRVulkan->gBuffer.normalTexture.allocation), nullptr));

		VkImageViewCreateInfo imageViewCreateInfo1 = rvkCreateImageViewCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT, pRVulkan->gBuffer.baseColorTexture.image, VK_IMAGE_ASPECT_COLOR_BIT);
		CHECK_VKRESULT(vkCreateImageView(pRVulkan->pDevice, &imageViewCreateInfo1, nullptr, &(pRVulkan->gBuffer.baseColorTexture.imageView)));

		VkImageViewCreateInfo imageViewCreateInfo2 = rvkCreateImageViewCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT, pRVulkan->gBuffer.normalTexture.image, VK_IMAGE_ASPECT_COLOR_BIT);
		CHECK_VKRESULT(vkCreateImageView(pRVulkan->pDevice, &imageViewCreateInfo2, nullptr, &pRVulkan->gBuffer.normalTexture.imageView));

		VkImageViewCreateInfo imageViewCreateInfo3 = rvkCreateImageViewCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT, pRVulkan->gBuffer.positionTexture.image, VK_IMAGE_ASPECT_COLOR_BIT);
		CHECK_VKRESULT(vkCreateImageView(pRVulkan->pDevice, &imageViewCreateInfo3, nullptr, &pRVulkan->gBuffer.positionTexture.imageView));

		//depth
		VkImageUsageFlags imageUsageFlags_depth = {};
		imageUsageFlags_depth |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageUsageFlags_depth |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageUsageFlags_depth |= VK_IMAGE_USAGE_SAMPLED_BIT;

		VkImageCreateInfo imageCreateInfo_depth = rvkCreateImageCreateInfo(VK_FORMAT_D32_SFLOAT, imageUsageFlags_depth, pRVulkan->drawImage.imageExtent);

		CHECK_VKRESULT(vmaCreateImage(pRVulkan->vmaAllocator, &imageCreateInfo_depth, &allocationCreateInfo, &pRVulkan->gBuffer.depthTexture.image, &pRVulkan->gBuffer.depthTexture.allocation, nullptr));

		VkImageViewCreateInfo imageViewCreateInfo_depth = rvkCreateImageViewCreateInfo(VK_FORMAT_D32_SFLOAT, pRVulkan->gBuffer.depthTexture.image, VK_IMAGE_ASPECT_DEPTH_BIT);
		CHECK_VKRESULT(vkCreateImageView(pRVulkan->pDevice, &imageViewCreateInfo_depth, nullptr, &pRVulkan->gBuffer.depthTexture.imageView));

		pRVulkan->deletionQueue.PushFunction([=]()
			{
				rvkDestroyImage(pRVulkan, pRVulkan->gBuffer.baseColorTexture);
				rvkDestroyImage(pRVulkan, pRVulkan->gBuffer.normalTexture);
				rvkDestroyImage(pRVulkan, pRVulkan->gBuffer.positionTexture);
				rvkDestroyImage(pRVulkan, pRVulkan->gBuffer.depthTexture);
			});
	}

	VkCommandBufferBeginInfo rvkCreateCommandBufferBeginInfo(VkCommandBufferUsageFlags flags)
	{
		VkCommandBufferBeginInfo commandBufferBeginInfo = {};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.flags = flags;
		return commandBufferBeginInfo;
	}


	void rvkImageLayoutTransition(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout)
	{
		VkImageMemoryBarrier2 imageBarrier = {};
		imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
		imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
		imageBarrier.oldLayout = currentLayout;
		imageBarrier.newLayout = newLayout;
		//imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		//imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		VkImageAspectFlags aspectMask = {};
		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
		{
			aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		}

		else
		{
			aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		imageBarrier.subresourceRange = rvkGetImageSubresourceRange(aspectMask);
		imageBarrier.image = image;

		VkDependencyInfo depInfo{};
		depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		depInfo.pNext = nullptr;

		depInfo.imageMemoryBarrierCount = 1;
		depInfo.pImageMemoryBarriers = &imageBarrier;

		vkCmdPipelineBarrier2(cmdBuffer, &depInfo);
	}

	VkImageSubresourceRange rvkGetImageSubresourceRange(VkImageAspectFlags aspectMask)
	{
		VkImageSubresourceRange subImage = {};
		subImage.aspectMask = aspectMask;
		subImage.baseMipLevel = 0;
		subImage.levelCount = VK_REMAINING_MIP_LEVELS;
		subImage.baseArrayLayer = 0;
		subImage.layerCount = VK_REMAINING_ARRAY_LAYERS;

		return subImage;
	}

	VkCommandBufferSubmitInfo rvkCreateCommandBufferSubmitInfo(VkCommandBuffer cmdBuffer)
	{
		VkCommandBufferSubmitInfo cmdBufferSubmitInfo = {};
		cmdBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		cmdBufferSubmitInfo.commandBuffer = cmdBuffer;
		cmdBufferSubmitInfo.deviceMask = 0;

		return cmdBufferSubmitInfo;
	}

	VkSemaphoreSubmitInfo rvkCreateSemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore)
	{
		VkSemaphoreSubmitInfo semaphoreSubmitInfo = {};
		semaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		semaphoreSubmitInfo.pNext = nullptr;
		semaphoreSubmitInfo.semaphore = semaphore;
		semaphoreSubmitInfo.stageMask = stageMask;
		semaphoreSubmitInfo.deviceIndex = 0;
		semaphoreSubmitInfo.value = 1;

		return semaphoreSubmitInfo;
	}

	VkSubmitInfo2 rvkCreateQueueSubmitInfo(VkCommandBufferSubmitInfo* cmdBufferInfo, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo)
	{
		VkSubmitInfo2 submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		
		if(waitSemaphoreInfo != nullptr)
		{
			submitInfo.waitSemaphoreInfoCount = 1;
			submitInfo.pWaitSemaphoreInfos = waitSemaphoreInfo;
		}

		submitInfo.commandBufferInfoCount = 1;
		submitInfo.pCommandBufferInfos = cmdBufferInfo;


		if (signalSemaphoreInfo != nullptr)
		{
			submitInfo.signalSemaphoreInfoCount = 1;
			submitInfo.pSignalSemaphoreInfos = signalSemaphoreInfo;
		}

		return submitInfo;
	}

	VkImageCreateInfo rvkCreateImageCreateInfo(VkFormat format, VkImageUsageFlags imageUsageFlags, VkExtent3D extent)
	{
		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = format;
		imageCreateInfo.extent = extent;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = imageUsageFlags;

		return imageCreateInfo;
	}

	VkImageViewCreateInfo rvkCreateImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags)
	{
		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.pNext = nullptr;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.image = image;
		imageViewCreateInfo.format = format;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags;

		return imageViewCreateInfo;
	}

	void rvkCopyImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize)
	{
		VkImageBlit2 blitRegion = {};
		blitRegion.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
		blitRegion.srcOffsets[1].x = srcSize.width;
		blitRegion.srcOffsets[1].y = srcSize.height;
		blitRegion.srcOffsets[1].z = 1;
		blitRegion.dstOffsets[1].x = dstSize.width;
		blitRegion.dstOffsets[1].y = dstSize.height;
		blitRegion.dstOffsets[1].z = 1;
		blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blitRegion.srcSubresource.baseArrayLayer = 0;
		blitRegion.srcSubresource.layerCount = 1;
		blitRegion.srcSubresource.mipLevel = 0;
		blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blitRegion.dstSubresource.baseArrayLayer = 0;
		blitRegion.dstSubresource.layerCount = 1;
		blitRegion.dstSubresource.mipLevel = 0;

		VkBlitImageInfo2 blitInfo = {};
		blitInfo.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
		blitInfo.dstImage = destination;
		blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		blitInfo.srcImage = source;
		blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		blitInfo.filter = VK_FILTER_LINEAR;
		blitInfo.regionCount = 1;
		blitInfo.pRegions = &blitRegion;

		vkCmdBlitImage2(cmd, &blitInfo);
	}

	void rvkCreateEngineUIDescriptorPool(RVulkan* pRVulkan)
	{
		VkDescriptorPoolSize poolSizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = 1000;
		poolInfo.poolSizeCount = (uint32_t)std::size(poolSizes);
		poolInfo.pPoolSizes = poolSizes;

		CHECK_VKRESULT(vkCreateDescriptorPool(pRVulkan->pDevice, &poolInfo, nullptr, &(pRVulkan->immediateUIPool)));

		pRVulkan->deletionQueue.PushFunction([=]()
			{
				vkDestroyDescriptorPool(pRVulkan->pDevice, pRVulkan->immediateUIPool, nullptr);
			});

		RLOG("[VULKAN] Immediate ui descriptor pool created.");
	}

	VkRenderingAttachmentInfo rvkCreateRenderingAttachmentInfo(VkImageView view, VkClearValue* clear, VkImageLayout layout)
	{
		VkRenderingAttachmentInfo colorAttachmentInfo = {};
		colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		colorAttachmentInfo.imageView = view;
		colorAttachmentInfo.imageLayout = layout;
		colorAttachmentInfo.loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		if (clear) 
		{
			colorAttachmentInfo.clearValue = *clear;
		}

		return colorAttachmentInfo;
	}

	VkRenderingInfo rvkCreateRenderingInfo(VkExtent2D renderExtent, VkRenderingAttachmentInfo* colorAttachments, uint32_t colorAttachmentCount, VkRenderingAttachmentInfo* depthAttachment)
	{
		VkRenderingInfo renderingInfo = {};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderingInfo.renderArea = VkRect2D{ VkOffset2D { 0, 0 }, renderExtent };
		renderingInfo.layerCount = 1;
		renderingInfo.colorAttachmentCount = colorAttachmentCount;
		renderingInfo.pColorAttachments = colorAttachments;
		renderingInfo.pDepthAttachment = depthAttachment;
		renderingInfo.pStencilAttachment = nullptr;

		return renderingInfo;
	}

	VkPipelineShaderStageCreateInfo rvkCreatePipelineShaderStageCreateInfo(VkShaderModule shaderModule, VkShaderStageFlagBits shaderStageFlag)
	{
		VkPipelineShaderStageCreateInfo stageCreateInfo = {};
		stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stageCreateInfo.stage = shaderStageFlag;
		stageCreateInfo.module = shaderModule;
		stageCreateInfo.pName = "main";

		return stageCreateInfo;
	}

	RVkAllocatedBuffer rvkCreateBuffer(RVulkan* pRulkan, size_t allocSize, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage)
	{
		VkBufferCreateInfo bufferCreateInfo = {};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = allocSize;
		bufferCreateInfo.usage = usageFlags;

		VmaAllocationCreateInfo allocCreateInfo = {};
		allocCreateInfo.usage = memoryUsage;
		allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		RVkAllocatedBuffer buffer;
		CHECK_VKRESULT(vmaCreateBuffer(pRulkan->vmaAllocator, &bufferCreateInfo, &allocCreateInfo, &buffer.vkBuffer, &buffer.vmaAllocation, &buffer.vmaAllocationInfo));

		return buffer;
	}

	void rvkDestoryBuffer(RVulkan* pRulkan, RVkAllocatedBuffer& buffer)
	{
		vmaDestroyBuffer(pRulkan->vmaAllocator, buffer.vkBuffer, buffer.vmaAllocation);
	}

	RVkMeshBuffer rvkCreateMeshBuffer(RVulkan* pRVulkan, std::vector<Vertex> vertices, std::vector<uint32_t> indices)
	{
		const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
		const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

		RVkMeshBuffer meshBuffer;
		meshBuffer.indexCount = indices.size();

		meshBuffer.vertexBuffer = rvkCreateBuffer(pRVulkan, vertexBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

		VkBufferDeviceAddressInfo deviceAdressInfo{ .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,.buffer = meshBuffer.vertexBuffer.vkBuffer };
		meshBuffer.vertexBufferAddress = vkGetBufferDeviceAddress(pRVulkan->pDevice, &deviceAdressInfo);

		meshBuffer.indexBuffer = rvkCreateBuffer(pRVulkan, indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY);

		RVkAllocatedBuffer staging = rvkCreateBuffer(pRVulkan, vertexBufferSize + indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

		void* data;
		vmaMapMemory(pRVulkan->vmaAllocator, staging.vmaAllocation, &data);

		memcpy(data, vertices.data(), vertexBufferSize);
		memcpy((char*)data + vertexBufferSize, indices.data(), indexBufferSize);

		vmaUnmapMemory(pRVulkan->vmaAllocator, staging.vmaAllocation);

		rvkImmediateSubmit(pRVulkan, [&](VkCommandBuffer cmd)
			{
			VkBufferCopy vertexCopy{ 0 };
			vertexCopy.dstOffset = 0;
			vertexCopy.srcOffset = 0;
			vertexCopy.size = vertexBufferSize;

			vkCmdCopyBuffer(cmd, staging.vkBuffer, meshBuffer.vertexBuffer.vkBuffer, 1, &vertexCopy);

			VkBufferCopy indexCopy{ 0 };
			indexCopy.dstOffset = 0;
			indexCopy.srcOffset = vertexBufferSize;
			indexCopy.size = indexBufferSize;

			vkCmdCopyBuffer(cmd, staging.vkBuffer, meshBuffer.indexBuffer.vkBuffer, 1, &indexCopy);
			});

		rvkDestoryBuffer(pRVulkan, staging);

		return meshBuffer;
	}

	void DestroyMeshBuffer(RVulkan* pRVulkan, RVkMeshBuffer& meshBuffer)
	{
		rvkDestoryBuffer(pRVulkan, meshBuffer.vertexBuffer);
		rvkDestoryBuffer(pRVulkan, meshBuffer.indexBuffer);
	}

	void rvkImmediateSubmit(RVulkan* pRVulkan, std::function<void(VkCommandBuffer cmd)>&& function)
	{
		CHECK_VKRESULT(vkResetFences(pRVulkan->pDevice, 1, &(pRVulkan->immediateFence)));
		CHECK_VKRESULT(vkResetCommandBuffer(pRVulkan->immediateCommandBuffer, 0));

		VkCommandBuffer cmdBuffer = pRVulkan->immediateCommandBuffer;

		VkCommandBufferBeginInfo cmdBeginInfo = rvkCreateCommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		CHECK_VKRESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo));

		function(cmdBuffer);

		CHECK_VKRESULT(vkEndCommandBuffer(cmdBuffer));

		VkCommandBufferSubmitInfo cmdinfo = rvkCreateCommandBufferSubmitInfo(cmdBuffer);
		VkSubmitInfo2 submit = rvkCreateQueueSubmitInfo(&cmdinfo, nullptr, nullptr);

		CHECK_VKRESULT(vkQueueSubmit2(pRVulkan->pGraphicsQueue, 1, &submit, pRVulkan->immediateFence));

		CHECK_VKRESULT(vkWaitForFences(pRVulkan->pDevice, 1, &(pRVulkan->immediateFence), true, UINT32_MAX));
	}

	VkRenderingAttachmentInfo  rvkCreateDepthRenderingAttachmentInfo(VkImageView view, VkImageLayout layout)
	{
		VkRenderingAttachmentInfo depthAttachment{};
		depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		depthAttachment.pNext = nullptr;

		depthAttachment.imageView = view;
		depthAttachment.imageLayout = layout;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.clearValue.depthStencil.depth = 0.f;

		return depthAttachment;
	}

	void rvkResizeSwapChain(RVulkan* pRVulkan)
	{
		vkDeviceWaitIdle(pRVulkan->pDevice);

		vkDestroySwapchainKHR(pRVulkan->pDevice, pRVulkan->pSwapChain, nullptr);
		rvkCreateSwapChain(pRVulkan);
	}

	RVkAllocatedImage rvkCreateImage(RVulkan* pRVulkan, VkExtent3D extent, VkFormat format, VkImageUsageFlags usageFlags, bool mipmapped)
	{
		RVkAllocatedImage image;
		image.imageFormat = format;
		image.imageExtent = extent;

		VkImageCreateInfo imgInfo = rvkCreateImageCreateInfo(format, usageFlags, extent);

		if (mipmapped) 
		{
			imgInfo.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(extent.width, extent.height)))) + 1;
		}

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		allocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		CHECK_VKRESULT(vmaCreateImage(pRVulkan->vmaAllocator, &imgInfo, &allocInfo, &(image.image), &(image.allocation), nullptr));
		
		VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;

		if (format == VK_FORMAT_D32_SFLOAT) 
		{
			aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
		}

		VkImageViewCreateInfo imageViewInfo = rvkCreateImageViewCreateInfo(format, image.image, aspectFlag);
		imageViewInfo.subresourceRange.levelCount = imgInfo.mipLevels;

		CHECK_VKRESULT(vkCreateImageView(pRVulkan->pDevice, &imageViewInfo, nullptr, &(image.imageView)));

		return image;
	}

	RVkAllocatedImage rvkCreateImage(RVulkan* pRVulkan, void* data, VkExtent3D extent, VkFormat format, VkImageUsageFlags usageFlags, bool mipmapped)
	{
		uint32_t data_size = extent.depth * extent.width * extent.height * 4;
		RVkAllocatedBuffer stgBuffer = rvkCreateBuffer(pRVulkan, data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		memcpy(stgBuffer.vmaAllocationInfo.pMappedData, data, data_size);

		RVkAllocatedImage image = rvkCreateImage(pRVulkan, extent, format, usageFlags, mipmapped);

		rvkImmediateSubmit(pRVulkan, [&](VkCommandBuffer cmdBuffer) 
			{
			rvkImageLayoutTransition(cmdBuffer, image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

			VkBufferImageCopy copyRegion = {};
			copyRegion.bufferOffset = 0;
			copyRegion.bufferRowLength = 0;
			copyRegion.bufferImageHeight = 0;

			copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.imageSubresource.mipLevel = 0;
			copyRegion.imageSubresource.baseArrayLayer = 0;
			copyRegion.imageSubresource.layerCount = 1;
			copyRegion.imageExtent = extent;

			vkCmdCopyBufferToImage(cmdBuffer, stgBuffer.vkBuffer, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

			rvkImageLayoutTransition(cmdBuffer, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			});

		rvkDestoryBuffer(pRVulkan, stgBuffer);

		return image;
	}

	void rvkDestroyImage(RVulkan* pRVulkan, const RVkAllocatedImage& img)
	{
		vkDestroyImageView(pRVulkan->pDevice, img.imageView, nullptr);
		vmaDestroyImage(pRVulkan->vmaAllocator, img.image, img.allocation);
	}

	VkSampler rvkCreateSamplerNearest(RVulkan* pRVulkan)
	{
		VkSamplerCreateInfo samplerCreateInfo = {};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
		samplerCreateInfo.minFilter = VK_FILTER_NEAREST;

		VkSampler sampler;
		vkCreateSampler(pRVulkan->pDevice, &samplerCreateInfo, nullptr, &sampler);
		return sampler;
	}

	VkSampler rvkCreateSamplerLinear(RVulkan* pRVulkan)
	{
		VkSamplerCreateInfo samplerCreateInfo = {};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.minFilter = VK_FILTER_LINEAR;

		VkSampler sampler;
		vkCreateSampler(pRVulkan->pDevice, &samplerCreateInfo, nullptr, &sampler);
		return sampler;
	}

	void rvkDestroySampler(RVulkan* pRVulkan, VkSampler sampler)
	{
		vkDestroySampler(pRVulkan->pDevice, sampler, nullptr);
	}


	void rvkWaitFences(RVulkan* pRVulkan, VkFence fence)
	{
		CHECK_VKRESULT_DEBUG(vkWaitForFences(pRVulkan->pDevice, 1, &fence, VK_TRUE, UINT64_MAX));
		CHECK_VKRESULT_DEBUG(vkResetFences(pRVulkan->pDevice, 1, &fence));
	}

	void RVkDescriptorLayoutBuilder::AddBinding(uint32_t binding, VkDescriptorType descriptorType)
	{
		VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {};
		descriptorSetLayoutBinding.binding = binding;
		descriptorSetLayoutBinding.descriptorType = descriptorType;
		descriptorSetLayoutBinding.descriptorCount = 1;

		bindings.push_back(descriptorSetLayoutBinding);
	}

	void RVkDescriptorLayoutBuilder::Clear()
	{
		bindings.clear();
	}

	VkDescriptorSetLayout RVkDescriptorLayoutBuilder::Build(RVulkan* pRVulkan, VkShaderStageFlags shaderStageFlags, void* pNext, VkDescriptorSetLayoutCreateFlags dcsSetLayoutCreateflags)
	{
		for(int i = 0; i < bindings.size(); i++)
		{
			bindings[i].stageFlags |= shaderStageFlags;
		}

		VkDescriptorSetLayoutCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.pNext = pNext;
		createInfo.flags = dcsSetLayoutCreateflags;
		createInfo.bindingCount = (uint32_t)bindings.size();
		createInfo.pBindings = bindings.data();

		VkDescriptorSetLayout setLayout;

		CHECK_VKRESULT(vkCreateDescriptorSetLayout(pRVulkan->pDevice, &createInfo, nullptr, &setLayout));

		return setLayout;
	}

	VkPipeline RVkPipelineBuilder::BuildPipeline(RVulkan* pRVulkan)
	{
		VkPipelineViewportStateCreateInfo viewPortStateCreateInfo = {};
		viewPortStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewPortStateCreateInfo.viewportCount = 1;
		viewPortStateCreateInfo.scissorCount = 1;


		VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
		colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
		colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;

		VkPipelineColorBlendAttachmentState colorBlendAttachmentStates[] = { colorBlendAttachmentState1 , colorBlendAttachmentState2 , colorBlendAttachmentState3 };
		colorBlendStateCreateInfo.attachmentCount = 3;
		colorBlendStateCreateInfo.pAttachments = colorBlendAttachmentStates;

		VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
		vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
		graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphicsPipelineCreateInfo.pNext = &pipelineRenderingCreateInfo;
		graphicsPipelineCreateInfo.stageCount = shaderStages.size();
		graphicsPipelineCreateInfo.pStages = shaderStages.data();
		graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
		graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
		graphicsPipelineCreateInfo.pViewportState = &viewPortStateCreateInfo;
		graphicsPipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
		graphicsPipelineCreateInfo.pMultisampleState = &multisamplingCreateInfo;
		graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
		graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
		graphicsPipelineCreateInfo.layout = pipelineLayout;

		VkDynamicState dyanmicState[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
		dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCreateInfo.pDynamicStates = &dyanmicState[0];
		dynamicStateCreateInfo.dynamicStateCount = 2;

		graphicsPipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;

		VkPipeline pPipeline;
		CHECK_VKRESULT(vkCreateGraphicsPipelines(pRVulkan->pDevice, nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &pPipeline));

		return pPipeline;
	}

	void RVkPipelineBuilder::Clear()
	{
		shaderStages.clear();
		inputAssemblyCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		rasterizerCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		colorBlendAttachmentState1 = {};
		colorBlendAttachmentState2 = {};
		colorBlendAttachmentState3 = {};
		multisamplingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		pipelineLayout = {};
		depthStencilCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
		pipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
	}

	void RVkPipelineBuilder::SetShaders(VkShaderModule vertexShader, VkShaderModule fragmentShader)
	{
		shaderStages.clear();
		shaderStages.push_back(rvkCreatePipelineShaderStageCreateInfo(vertexShader, VK_SHADER_STAGE_VERTEX_BIT));
		shaderStages.push_back(rvkCreatePipelineShaderStageCreateInfo(fragmentShader, VK_SHADER_STAGE_FRAGMENT_BIT));
	}

	void RVkPipelineBuilder::SetInputTopology(VkPrimitiveTopology topology)
	{
		inputAssemblyCreateInfo.topology = topology;
		inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;
	}

	void RVkPipelineBuilder::SetPolygonMode(VkPolygonMode polygonMode)
	{
		rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizerCreateInfo.polygonMode = polygonMode;
		rasterizerCreateInfo.lineWidth = 1.0f;
	}

	void RVkPipelineBuilder::SetCullMode(VkCullModeFlags cullModeFlags, VkFrontFace frontFace)
	{
		rasterizerCreateInfo.cullMode = cullModeFlags;
		rasterizerCreateInfo.frontFace = frontFace;
	}

	void RVkPipelineBuilder::SetMultisampling(bool b)
	{
		if(b == false)
		{
			multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;
			multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			multisamplingCreateInfo.minSampleShading = 1.0f;
			multisamplingCreateInfo.pSampleMask = nullptr;
			multisamplingCreateInfo.alphaToCoverageEnable = VK_FALSE;
			multisamplingCreateInfo.alphaToOneEnable = VK_FALSE;
		}
	}

	void RVkPipelineBuilder::SetBlending(bool b)
	{
		if(b == false)
		{
			colorBlendAttachmentState1.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachmentState1.blendEnable = VK_FALSE;

			colorBlendAttachmentState2.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachmentState2.blendEnable = VK_FALSE;

			colorBlendAttachmentState3.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachmentState3.blendEnable = VK_FALSE;
		}
	}

	void RVkPipelineBuilder::SetColorAttachmentFormat(VkFormat* colorAttachmentFormats, uint32_t colorAttachmentCount)
	{
		pipelineRenderingCreateInfo.colorAttachmentCount = colorAttachmentCount;
		pipelineRenderingCreateInfo.pColorAttachmentFormats = colorAttachmentFormats;
	}

	void RVkPipelineBuilder::SetDepthFormat(VkFormat format)
	{
		pipelineRenderingCreateInfo.depthAttachmentFormat = format;
	}

	void RVkPipelineBuilder::SetDepthTest(bool b, VkCompareOp op)
	{
		if(b == false)
		{
			depthStencilCreateInfo.depthTestEnable = VK_FALSE;
			depthStencilCreateInfo.depthWriteEnable = VK_FALSE;
			depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_NEVER;
			depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
			depthStencilCreateInfo.stencilTestEnable = VK_FALSE;
			depthStencilCreateInfo.front = {};
			depthStencilCreateInfo.back = {};
			depthStencilCreateInfo.minDepthBounds = 0.f;
			depthStencilCreateInfo.maxDepthBounds = 1.f;
		}

		if(b == true)
		{
			depthStencilCreateInfo.depthTestEnable = VK_TRUE;
			depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
			depthStencilCreateInfo.depthCompareOp = op;
			depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
			depthStencilCreateInfo.stencilTestEnable = VK_FALSE;
			depthStencilCreateInfo.front = {};
			depthStencilCreateInfo.back = {};
			depthStencilCreateInfo.minDepthBounds = 0.f;
			depthStencilCreateInfo.maxDepthBounds = 1.f;
		}
	}

	void RVkDescriptorWriter::WriteBuffer(uint32_t binding, VkDescriptorType descriptorType, VkBuffer buffer, size_t range, size_t offset)
	{
		VkDescriptorBufferInfo& bufferInfo = bufferInfos.emplace_back(VkDescriptorBufferInfo{
		.buffer = buffer,
		.offset = offset,
		.range = range
			});

		VkWriteDescriptorSet writeDescriptorSet = {};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = VK_NULL_HANDLE;
		writeDescriptorSet.dstBinding = binding;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = descriptorType;
		writeDescriptorSet.pBufferInfo = &bufferInfo;

		writes.push_back(writeDescriptorSet);
	}

	void RVkDescriptorWriter::WriteImage(uint32_t binding, VkDescriptorType descriptorType, VkImageView imageView, VkSampler sampler, VkImageLayout layout)
	{
		VkDescriptorImageInfo& info = imageInfos.emplace_back(VkDescriptorImageInfo{
		.sampler = sampler,
		.imageView = imageView,
		.imageLayout = layout
			});

		VkWriteDescriptorSet writeDescriptorSet = {};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = VK_NULL_HANDLE;
		writeDescriptorSet.dstBinding = binding;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = descriptorType;
		writeDescriptorSet.pImageInfo = &info;

		writes.push_back(writeDescriptorSet);

	}

	void RVkDescriptorWriter::UpdateDescriptorSets(RVulkan* pRVulkan, VkDescriptorSet dstSet)
	{
		for (VkWriteDescriptorSet& write : writes) 
		{
			write.dstSet = dstSet;
		}

		vkUpdateDescriptorSets(pRVulkan->pDevice, writes.size(), writes.data(), 0, nullptr);
	}

	void RVkDescriptorWriter::Clear()
	{
		imageInfos.clear();
		writes.clear();
		bufferInfos.clear();
	}

	void RVkDescriptorAllocator::InitPool(RVulkan* pRVulkan, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios)
	{
		ratios.clear();

		for (auto poolRatio : poolRatios)
		{
			ratios.push_back(poolRatio);
		}

		VkDescriptorPool newPool = CreatePool(pRVulkan, maxSets, poolRatios);

		setsPerPool = maxSets * 1.5;

		emptyPools.push_back(newPool);
	}

	void RVkDescriptorAllocator::ClearDescriptors(RVulkan* pRVulkan)
	{
		for (auto pool : emptyPools) 
		{
			vkResetDescriptorPool(pRVulkan->pDevice, pool, 0);
		}

		for (auto pool : fullPools)
		{
			vkResetDescriptorPool(pRVulkan->pDevice, pool, 0);
			emptyPools.push_back(pool);
		}

		fullPools.clear();
	}

	void RVkDescriptorAllocator::DestroyPool(RVulkan* pRVulkan)
	{
		for (auto pool : emptyPools)
		{
			vkDestroyDescriptorPool(pRVulkan->pDevice, pool, nullptr);
		}

		emptyPools.clear();

		for (auto pool : fullPools)
		{
			vkDestroyDescriptorPool(pRVulkan->pDevice, pool, nullptr);
		}

		fullPools.clear();
	}

	VkDescriptorSet RVkDescriptorAllocator::Allocate(RVulkan* pRVulkan, VkDescriptorSetLayout layout, void* pNext)
	{
		VkDescriptorPool poolToUse = GetPool(pRVulkan);

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.pNext = pNext;
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = poolToUse;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &layout;

		VkDescriptorSet descriptorSet;
		VkResult result = vkAllocateDescriptorSets(pRVulkan->pDevice, &allocInfo, &descriptorSet);

		if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL) {

			fullPools.push_back(poolToUse);

			poolToUse = GetPool(pRVulkan);
			allocInfo.descriptorPool = poolToUse;

			CHECK_VKRESULT(vkAllocateDescriptorSets(pRVulkan->pDevice, &allocInfo, &descriptorSet));
		}

		emptyPools.push_back(poolToUse);
		return descriptorSet;
	}

	VkDescriptorPool RVkDescriptorAllocator::GetPool(RVulkan* pRVulkan)
	{
		VkDescriptorPool pool;
		
		if(emptyPools.size() != 0)
		{
			pool = emptyPools.back();
			emptyPools.pop_back();
		}

		else
		{
			pool = CreatePool(pRVulkan, setsPerPool, ratios);

			setsPerPool = setsPerPool * 1.5;
			if (setsPerPool > 4092) {
				setsPerPool = 4092;
			}
		}

		return pool;
	}

	VkDescriptorPool RVkDescriptorAllocator::CreatePool(RVulkan* pRVulkan, uint32_t setCount, std::span<PoolSizeRatio> poolRatios)
	{
		std::vector<VkDescriptorPoolSize> poolSizes;

		for (PoolSizeRatio ratio : poolRatios) 
		{
			poolSizes.push_back(VkDescriptorPoolSize
				{
				.type = ratio.descriptorType,
				.descriptorCount = uint32_t(ratio.ratio * setCount)
				});
		}

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
		descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCreateInfo.flags = 0;
		descriptorPoolCreateInfo.maxSets = setCount;
		descriptorPoolCreateInfo.poolSizeCount = (uint32_t)poolSizes.size();
		descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();

		VkDescriptorPool pool;
		vkCreateDescriptorPool(pRVulkan->pDevice, &descriptorPoolCreateInfo, nullptr, &pool);
		return pool;
	}

	void RVkFrameData::ResetFrame(RVulkan* pRVulkan)
	{
		deletionQueue.flush();
		CHECK_VKRESULT_DEBUG(vkResetCommandBuffer(mainCommandBuffer, 0));
	}
	uint32_t RVkFrameData::GetNextImage(RVulkan* pRVulkan)
	{
		uint32_t imageIndex;
		VkResult nextImageResult = vkAcquireNextImageKHR(pRVulkan->pDevice, pRVulkan->pSwapChain, UINT64_MAX, swapchainSemaphore, nullptr, &imageIndex);

		if (nextImageResult == VK_ERROR_OUT_OF_DATE_KHR)
		{
			pRVulkan->resizeRequest = true;
			return -1;
		}

		return imageIndex;
	}

	void RVkFrameData::BeginFrame(RVulkan* pRVulkan)
	{
		VkCommandBufferBeginInfo cmdBufferbeginInfo = rvkCreateCommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		CHECK_VKRESULT_DEBUG(vkBeginCommandBuffer(mainCommandBuffer, &cmdBufferbeginInfo));

		VkClearColorValue clearValue = { { 0.0f, 0.0f, 0.0f, 1.0f } };
		VkImageSubresourceRange clearRange = rvkGetImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
		
		//main framebuffer
		rvkImageLayoutTransition(mainCommandBuffer, pRVulkan->swapChainImages[pRVulkan->currentImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
		vkCmdClearColorImage(mainCommandBuffer, pRVulkan->swapChainImages[pRVulkan->currentImageIndex], VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
		rvkImageLayoutTransition(mainCommandBuffer, pRVulkan->swapChainImages[pRVulkan->currentImageIndex], VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		//g buffers
		rvkImageLayoutTransition(mainCommandBuffer, pRVulkan->gBuffer.baseColorTexture.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
		vkCmdClearColorImage(mainCommandBuffer, pRVulkan->gBuffer.baseColorTexture.image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
		rvkImageLayoutTransition(mainCommandBuffer, pRVulkan->gBuffer.baseColorTexture.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		rvkImageLayoutTransition(mainCommandBuffer, pRVulkan->gBuffer.positionTexture.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
		vkCmdClearColorImage(mainCommandBuffer, pRVulkan->gBuffer.positionTexture.image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
		rvkImageLayoutTransition(mainCommandBuffer, pRVulkan->gBuffer.positionTexture.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		rvkImageLayoutTransition(mainCommandBuffer, pRVulkan->gBuffer.normalTexture.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
		vkCmdClearColorImage(mainCommandBuffer, pRVulkan->gBuffer.normalTexture.image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
		rvkImageLayoutTransition(mainCommandBuffer, pRVulkan->gBuffer.normalTexture.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		//depth
		rvkImageLayoutTransition(mainCommandBuffer, pRVulkan->gBuffer.depthTexture.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

		//drawImage
		rvkImageLayoutTransition(mainCommandBuffer, pRVulkan->drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
		//vkCmdClearColorImage(mainCommandBuffer, pRVulkan->drawImage.image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
	}

	void RVkFrameData::EndFrame(RVulkan* pRVulkan)
	{
		//drawImageRender
		rvkImageLayoutTransition(mainCommandBuffer, pRVulkan->drawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		rvkImageLayoutTransition(mainCommandBuffer, pRVulkan->gBuffer.baseColorTexture.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		rvkCopyImageToImage(mainCommandBuffer, pRVulkan->gBuffer.baseColorTexture.image, pRVulkan->drawImage.image, pRVulkan->drawExtent, pRVulkan->drawExtent);
		rvkImageLayoutTransition(mainCommandBuffer, pRVulkan->drawImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);

		CHECK_VKRESULT_DEBUG(vkEndCommandBuffer(mainCommandBuffer));
	}

	void RVkFrameData::SubmitCommandBuffer(RVulkan* pRVulkan)
	{
		VkCommandBufferSubmitInfo cmdBufferSubmitInfo = rvkCreateCommandBufferSubmitInfo(mainCommandBuffer);
		VkSemaphoreSubmitInfo waitSempSubmitInfo = rvkCreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, swapchainSemaphore);
		VkSemaphoreSubmitInfo signalSempSubmitInfo = rvkCreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, renderSemaphore);
		VkSubmitInfo2 submitInfo = rvkCreateQueueSubmitInfo(&cmdBufferSubmitInfo, &signalSempSubmitInfo, &waitSempSubmitInfo);

		CHECK_VKRESULT_DEBUG(vkQueueSubmit2(pRVulkan->pGraphicsQueue, 1, &submitInfo, renderFence));
	}

	void RVkMeshBuffer::Destroy(RVulkan* pRVulkan)
	{
		rvkDestoryBuffer(pRVulkan, vertexBuffer);
		rvkDestoryBuffer(pRVulkan, indexBuffer);
	}
}