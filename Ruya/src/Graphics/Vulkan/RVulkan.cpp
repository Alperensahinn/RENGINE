#include "RVulkan.h"
#include "../../Utilities/FileSystem/FileSystem.h"
#include "../../Utilities/Log/RLog.h"

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cmath>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace Ruya 
{
	RVulkan::RVulkan(GLFWwindow& window)
	{
		Init(window);
	}

	RVulkan::~RVulkan()
	{
		CleanUp();
	}

	void RVulkan::Init(GLFWwindow& window)
	{
		rVk::CreateInstance(this);
		rVk::CreateDebugMessenger(this);
		rVk::CreateWindowSurface(this, window);
		rVk::SelectPhysicalDevice(this);
		rVk::CheckQueueFamilies(this);
		rVk::CreateDevice(this);
		rVk::CreateVulkanMemoryAllocator(this);
		rVk::CreateSwapChain(this, window);
		rVk::SetQueues(this);
		rVk::CreateRenderPass(this);
		rVk::CreateGraphicsPipeline(this);
		rVk::CreateFrameBuffers(this);
		rVk::CreateCommandPool(this);
		rVk::CreateSynchronizationObjects(this);
	}

	void RVulkan::CleanUp()
	{
		vkDeviceWaitIdle(pDevice);

		mainDeletionQueue.flush();

		for (int i = 0; i < frameOverlap; i++)
		{
			vkDestroyCommandPool(pDevice, frames[i].commandPool, nullptr);
			vkDestroySemaphore(pDevice, frames[i].swapchainSemaphore, nullptr);
			vkDestroySemaphore(pDevice, frames[i].renderSemaphore, nullptr);
			vkDestroyFence(pDevice, frames[i].renderFence, nullptr);
		}

		for (int i = 0; i < swapChainFramebuffers.size(); i++)
		{
			vkDestroyFramebuffer(pDevice, swapChainFramebuffers[i], nullptr);
		}

		vkDestroyPipeline(pDevice, pGraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(pDevice, pPipelineLayout, nullptr);
		vkDestroyRenderPass(pDevice, pRenderPass, nullptr);

		for (int i = 0; i < swapChainImageViews.size(); i++)
		{
			vkDestroyImageView(pDevice, swapChainImageViews[i], nullptr);
		}

		vkDestroySwapchainKHR(pDevice, pSwapChain, nullptr);
		vkDestroyDevice(pDevice, nullptr);

		if (enableValidationLayers)
		{
			rVk::DestroyDebugUtilsMessenger(this);
		}

		vkDestroySurfaceKHR(pInstance, pSurface, nullptr);
		vkDestroyInstance(pInstance, nullptr);
	}

	void RVulkan::Draw()
	{
		CHECK_VKRESULT_DEBUG(vkWaitForFences(pDevice, 1, &GetCurrentFrame().renderFence, VK_TRUE, UINT64_MAX));
		CHECK_VKRESULT_DEBUG(vkResetFences(pDevice, 1, &GetCurrentFrame().renderFence));

		uint32_t imageIndex;
		CHECK_VKRESULT_DEBUG(vkAcquireNextImageKHR(pDevice, pSwapChain, UINT64_MAX, GetCurrentFrame().swapchainSemaphore, nullptr, &imageIndex));

		VkCommandBuffer cmdBuffer = GetCurrentFrame().mainCommandBuffer;
		CHECK_VKRESULT_DEBUG(vkResetCommandBuffer(cmdBuffer, 0));

		VkCommandBufferBeginInfo cmdBufferbeginInfo = rVk::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);


		drawExtent.width = drawImage.imageExtent.width;
		drawExtent.height = drawImage.imageExtent.height;

		CHECK_VKRESULT_DEBUG(vkBeginCommandBuffer(cmdBuffer, &cmdBufferbeginInfo));

		rVk::TransitionImage(cmdBuffer, drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

		VkClearColorValue clearValue;
		float flash = std::abs(std::sin(frameNumber / 120.f));
		clearValue = { { 0.0f, 0.0f, flash, 1.0f } };

		VkImageSubresourceRange clearRange = rVk::ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

		vkCmdClearColorImage(cmdBuffer, drawImage.image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

		rVk::TransitionImage(cmdBuffer, drawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		rVk::TransitionImage(cmdBuffer, swapChainImages[imageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		rVk::CopyImageToImage(cmdBuffer, drawImage.image, swapChainImages[imageIndex], drawExtent, swapChainExtent);

		rVk::TransitionImage(cmdBuffer, swapChainImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		CHECK_VKRESULT_DEBUG(vkEndCommandBuffer(cmdBuffer));

		VkCommandBufferSubmitInfo cmdBufferSubmitInfo = rVk::CommandBufferSubmitInfo(cmdBuffer);
		VkSemaphoreSubmitInfo waitSempSubmitInfo = rVk::SemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, GetCurrentFrame().swapchainSemaphore);
		VkSemaphoreSubmitInfo signalSempSubmitInfo = rVk::SemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, GetCurrentFrame().renderSemaphore);
		VkSubmitInfo2 submitInfo = rVk::SubmitInfo(&cmdBufferSubmitInfo, &signalSempSubmitInfo, &waitSempSubmitInfo);

		CHECK_VKRESULT_DEBUG(vkQueueSubmit2(pGraphicsQueue, 1, &submitInfo, GetCurrentFrame().renderFence));

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.pSwapchains = &pSwapChain;
		presentInfo.swapchainCount = 1;

		presentInfo.pWaitSemaphores = &GetCurrentFrame().renderSemaphore;
		presentInfo.waitSemaphoreCount = 1;

		presentInfo.pImageIndices = &imageIndex;

		CHECK_VKRESULT_DEBUG(vkQueuePresentKHR(pGraphicsQueue, &presentInfo));

		frameNumber++;

		GetCurrentFrame().deletionQueue.flush();
	}

	RVkFrameData& RVulkan::GetCurrentFrame()
	{
		return frames[frameNumber % frameOverlap];
	}

	void rVk::CreateInstance(RVulkan* pRVulkan)
	{
		VkApplicationInfo applicationInfo = {};
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.apiVersion = VK_API_VERSION_1_3;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &applicationInfo;

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};


		if (pRVulkan->enableValidationLayers && !CheckValidationLayerSupport(pRVulkan))
		{
			RERRLOG("[VULKAN ERROR] Validation layers requested, but not available.")
		}

		if (pRVulkan->enableValidationLayers && CheckValidationLayerSupport(pRVulkan))
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

		RLOG("[VULKAN] Vulkan instance created.")
	}

	bool rVk::CheckValidationLayerSupport(RVulkan* pRVulkan)
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


	void rVk::CreateDebugMessenger(RVulkan* pRVulkan)
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
	}

	void rVk::DestroyDebugUtilsMessenger(RVulkan* pRVulkan)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(pRVulkan->pInstance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(pRVulkan->pInstance, pRVulkan->pDebugUtilsMessanger, nullptr);
		}
	}

	void rVk::SelectPhysicalDevice(RVulkan* pRVulkan)
	{
		unsigned int physicalDevicesCount;
		CHECK_VKRESULT(vkEnumeratePhysicalDevices(pRVulkan->pInstance, &physicalDevicesCount, nullptr));

		if (physicalDevicesCount == 0)
		{
			RERRLOG("[VULKAN ERROR] Failed to find Vulkan physical device.")
		}

		std::vector<VkPhysicalDevice> physicalDevices(physicalDevicesCount);
		CHECK_VKRESULT(vkEnumeratePhysicalDevices(pRVulkan->pInstance, &physicalDevicesCount, physicalDevices.data()));

		for (VkPhysicalDevice physicalDevice : physicalDevices)
		{
			VkPhysicalDeviceProperties physicalDeviceProperties;

			vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

			if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				pRVulkan->pPhysicalDevice = physicalDevice;
				break;
			}
		}

		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(pRVulkan->pPhysicalDevice, &physicalDeviceProperties);
		RLOG("[VULKAN] Vulkan physical device selected: " << physicalDeviceProperties.deviceName)
	}

	void rVk::CheckQueueFamilies(RVulkan* pRVulkan)
	{
		unsigned int queueFamilyPropertyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(pRVulkan->pPhysicalDevice, &queueFamilyPropertyCount, nullptr);

		if (queueFamilyPropertyCount == 0)
		{
			RERRLOG("[VULKAN ERROR] No queue families supported.")
		}

		std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(pRVulkan->pPhysicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());

		for (int i = 0; i < queueFamilyPropertyCount; i++)
		{
			if (queueFamilyProperties.at(i).queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				pRVulkan->graphicsQueueIndex = i;
			}
		}

		RLOG("[VULKAN] Queue family with graphics bit found. Index: " << pRVulkan->graphicsQueueIndex)
	}

	void rVk::CreateDevice(RVulkan* pRVulkan)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = pRVulkan->graphicsQueueIndex;
		queueCreateInfo.queueCount = 1;

		float queuePrioritie = 1.0f;
		queueCreateInfo.pQueuePriorities = &queuePrioritie;

		VkPhysicalDeviceFeatures physicalDeviceFutures = {};

		VkPhysicalDeviceVulkan13Features features13 = {};
		features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		features13.dynamicRendering = VK_TRUE;
		features13.synchronization2 = VK_TRUE;

		VkPhysicalDeviceFeatures2 features2 = {};
		features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features2.pNext = &features13;
		features2.features = physicalDeviceFutures;

		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pNext = &features2;
		deviceCreateInfo.queueCreateInfoCount = 1;
		deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
		deviceCreateInfo.pEnabledFeatures = nullptr;
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(pRVulkan->deviceExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = pRVulkan->deviceExtensions.data();

		CHECK_VKRESULT(vkCreateDevice(pRVulkan->pPhysicalDevice, &deviceCreateInfo, nullptr, &(pRVulkan->pDevice)));

		RLOG("[VULKAN] Logical device created.");
	}

	void rVk::SetQueues(RVulkan* pRVulkan)
	{
		vkGetDeviceQueue(pRVulkan->pDevice, pRVulkan->graphicsQueueIndex, 0, &(pRVulkan->pGraphicsQueue));
		vkGetDeviceQueue(pRVulkan->pDevice, pRVulkan->graphicsQueueIndex, 0, &(pRVulkan->pPresentQueue));

		RLOG("[VULKAN] Device queues setted.");
	}

	void rVk::CreateWindowSurface(RVulkan* pRVulkan, GLFWwindow& window)
	{
		VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.hwnd = glfwGetWin32Window(&window);
		surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);

		CHECK_VKRESULT(vkCreateWin32SurfaceKHR(pRVulkan->pInstance, &surfaceCreateInfo, nullptr, &(pRVulkan->pSurface)));

		RLOG("[VULKAN] Window surface created.")
	}

	void rVk::CreateSwapChain(RVulkan* pRVulkan, GLFWwindow& window)
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
			std::cerr << "[VULKAN ERROR] Device not support swap chain." << std::endl;
			throw std::runtime_error("[VULKAN ERROR] Device not support swap chain.");
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
		glfwGetFramebufferSize(&window, &width, &height);

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
		}


		VkExtent3D drawImageExtent = {
			actualExtent.width,
			actualExtent.height,
			1
		};

		pRVulkan->drawImage.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
		pRVulkan->drawImage.imageExtent = drawImageExtent;

		VkImageUsageFlags drawImageUsageFlags = {};
		drawImageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		drawImageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		drawImageUsageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
		drawImageUsageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		VkImageCreateInfo renderImageCreateInfo = ImageCreateInfo(pRVulkan->drawImage.imageFormat, drawImageUsageFlags, pRVulkan->drawImage.imageExtent);

		VmaAllocationCreateInfo rimgAllocCreateInfo = {};
		rimgAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		rimgAllocCreateInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		CHECK_VKRESULT(vmaCreateImage(pRVulkan->vmaAllocator, &renderImageCreateInfo, &rimgAllocCreateInfo, &(pRVulkan->drawImage.image), &(pRVulkan->drawImage.allocation), nullptr));

		VkImageViewCreateInfo rimgCreateInfo = ImageViewCreateInfo(pRVulkan->drawImage.imageFormat, pRVulkan->drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

		CHECK_VKRESULT(vkCreateImageView(pRVulkan->pDevice, &rimgCreateInfo, nullptr, &(pRVulkan->drawImage.imageView)));

		pRVulkan->mainDeletionQueue.PushFunction([=]()
			{
				vmaDestroyImage(pRVulkan->vmaAllocator, pRVulkan->drawImage.image, pRVulkan->drawImage.allocation);
				vkDestroyImageView(pRVulkan->pDevice, pRVulkan->drawImage.imageView, nullptr);
			});

		RLOG("[VULKAN] Swap chain created.");
	}

	void rVk::CreateGraphicsPipeline(RVulkan* pRVulkan)
	{
		VkShaderModule vertexShaderModule;
		VkShaderModule fragmentShaderModule;

		std::vector<char> vertexShaderCode = Ruya::ReadBinaryFile("src/Graphics/Shaders/vert.spv");
		std::vector<char> fragmentShaderCode = Ruya::ReadBinaryFile("src/Graphics/Shaders/frag.spv");

		vertexShaderModule = CreateShaderModule(pRVulkan, vertexShaderCode);
		fragmentShaderModule = CreateShaderModule(pRVulkan, fragmentShaderCode);

		VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo = {};
		vertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertexShaderStageCreateInfo.module = vertexShaderModule;
		vertexShaderStageCreateInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo = {};
		fragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragmentShaderStageCreateInfo.module = fragmentShaderModule;
		fragmentShaderStageCreateInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStageCreateInfos[] = { vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo };


		VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
		vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
		vertexInputStateCreateInfo.pVertexBindingDescriptions = nullptr;
		vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
		vertexInputStateCreateInfo.pVertexAttributeDescriptions = nullptr;


		VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
		inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;


		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)pRVulkan->swapChainExtent.width;
		viewport.height = (float)pRVulkan->swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = pRVulkan->swapChainExtent;

		VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
		viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateCreateInfo.viewportCount = 1;
		viewportStateCreateInfo.pViewports = &viewport;
		viewportStateCreateInfo.scissorCount = 1;
		viewportStateCreateInfo.pScissors = &scissor;


		VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo = {};
		rasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationCreateInfo.depthClampEnable = VK_FALSE;
		rasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationCreateInfo.lineWidth = 1.0f;
		rasterizationCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizationCreateInfo.depthBiasEnable = VK_FALSE;


		VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {};
		multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
		multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;


		VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
		colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachmentState.blendEnable = VK_FALSE;


		VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
		colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
		colorBlendStateCreateInfo.attachmentCount = 1;
		colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;


		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		CHECK_VKRESULT(vkCreatePipelineLayout(pRVulkan->pDevice, &pipelineLayoutCreateInfo, nullptr, &(pRVulkan->pPipelineLayout)));

		VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
		graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphicsPipelineCreateInfo.stageCount = 2;
		graphicsPipelineCreateInfo.pStages = shaderStageCreateInfos;
		graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
		graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
		graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
		graphicsPipelineCreateInfo.pRasterizationState = &rasterizationCreateInfo;
		graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
		graphicsPipelineCreateInfo.pDepthStencilState = nullptr;
		graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
		graphicsPipelineCreateInfo.layout = pRVulkan->pPipelineLayout;
		graphicsPipelineCreateInfo.renderPass = pRVulkan->pRenderPass;
		graphicsPipelineCreateInfo.subpass = 0;

		CHECK_VKRESULT(vkCreateGraphicsPipelines(pRVulkan->pDevice, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &(pRVulkan->pGraphicsPipeline)));

		RLOG("[VULKAN] Graphics pipeline created.")

		vkDestroyShaderModule(pRVulkan->pDevice, vertexShaderModule, nullptr);
		vkDestroyShaderModule(pRVulkan->pDevice, fragmentShaderModule, nullptr);
	}

	VkShaderModule rVk::CreateShaderModule(RVulkan* pRVulkan, std::vector<char>& shaderCode)
	{
		VkShaderModule shaderModule;

		VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
		shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.codeSize = shaderCode.size();
		shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

		CHECK_VKRESULT(vkCreateShaderModule(pRVulkan->pDevice, &shaderModuleCreateInfo, nullptr, &shaderModule));

		return shaderModule;
	}

	void rVk::CreateRenderPass(RVulkan* pRVulkan)
	{
		VkAttachmentDescription attachmentDescription = {};
		attachmentDescription.format = pRVulkan->swapChainImageFormat;
		attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference attachmentReference = {};
		attachmentReference.attachment = 0;
		attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &attachmentReference;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassCreateInfo = {};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = 1;
		renderPassCreateInfo.pAttachments = &attachmentDescription;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpassDescription;
		renderPassCreateInfo.dependencyCount = 1;
		renderPassCreateInfo.pDependencies = &dependency;

		CHECK_VKRESULT(vkCreateRenderPass(pRVulkan->pDevice, &renderPassCreateInfo, nullptr, &(pRVulkan->pRenderPass)));

		RLOG("[VULKAN] Render pass created.")
	}

	void rVk::CreateFrameBuffers(RVulkan* pRVulkan)
	{
		pRVulkan->swapChainFramebuffers.resize(pRVulkan->swapChainImageViews.size());

		for (int i = 0; i < pRVulkan->swapChainImageViews.size(); i++)
		{
			VkFramebufferCreateInfo frameBufferCreateInfo = {};
			frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frameBufferCreateInfo.renderPass = pRVulkan->pRenderPass;
			frameBufferCreateInfo.pAttachments = &(pRVulkan->swapChainImageViews[i]);
			frameBufferCreateInfo.attachmentCount = 1;
			frameBufferCreateInfo.width = pRVulkan->swapChainExtent.width;
			frameBufferCreateInfo.height = pRVulkan->swapChainExtent.height;
			frameBufferCreateInfo.layers = 1;

			CHECK_VKRESULT(vkCreateFramebuffer(pRVulkan->pDevice, &frameBufferCreateInfo, nullptr, &(pRVulkan->swapChainFramebuffers[i])));
		}

		RLOG("[VULKAN] Frame buffers created.")
	}

	void rVk::CreateCommandPool(RVulkan* pRVulkan)
	{
		VkCommandPoolCreateInfo commandPoolCreateInfo = {};
		commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolCreateInfo.queueFamilyIndex = pRVulkan->graphicsQueueIndex;

		for (int i = 0; i < frameOverlap; i++)
		{
			CHECK_VKRESULT(vkCreateCommandPool(pRVulkan->pDevice, &commandPoolCreateInfo, nullptr, &(pRVulkan->frames[i].commandPool)));

			VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
			commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			commandBufferAllocateInfo.commandPool = pRVulkan->frames[i].commandPool;
			commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			commandBufferAllocateInfo.commandBufferCount = 1;

			CHECK_VKRESULT(vkAllocateCommandBuffers(pRVulkan->pDevice, &commandBufferAllocateInfo, &(pRVulkan->frames[i].mainCommandBuffer)));
		}

		RLOG("[VULKAN] Command pools and buffers created.")
	}

	void rVk::RecordCommandBuffer(RVulkan* pRVulkan, VkCommandBuffer commandBuffer, uint32_t frameBufferIndex)
	{
		VkCommandBufferBeginInfo commandBufferBeginInfo = {};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		CHECK_VKRESULT(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = pRVulkan->pRenderPass;
		renderPassBeginInfo.framebuffer = pRVulkan->swapChainFramebuffers[frameBufferIndex];
		renderPassBeginInfo.renderArea.offset = { 0,0 };
		renderPassBeginInfo.renderArea.extent = pRVulkan->swapChainExtent;
		renderPassBeginInfo.clearValueCount = 1;

		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassBeginInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pRVulkan->pGraphicsPipeline);

		vkCmdDraw(commandBuffer, 3, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		CHECK_VKRESULT_DEBUG(vkEndCommandBuffer(commandBuffer));
	}

	void rVk::CreateSynchronizationObjects(RVulkan* pRVulkan)
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
		}

		RLOG("[VULKAN] Synchronization objects created.")
	}

	void rVk::CreateVulkanMemoryAllocator(RVulkan* pRVulkan)
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

		pRVulkan->mainDeletionQueue.PushFunction([vmaAllocator = pRVulkan->vmaAllocator]() {vmaDestroyAllocator(vmaAllocator);});

		RLOG("[VULKAN] Vulkan memory allocator created.")
	}

	void rVk::CreateBuffer()
	{
		VkBufferCreateInfo bufferCreateInfo = {};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;


		//vmaCreateBuffer(vmaAllocator, );
	}

	VkCommandBufferBeginInfo rVk::CommandBufferBeginInfo(VkCommandBufferUsageFlags flags)
	{
		VkCommandBufferBeginInfo commandBufferBeginInfo = {};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.flags = flags;
		return commandBufferBeginInfo;
	}


	void rVk::TransitionImage(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout)
	{
		VkImageMemoryBarrier2 imageBarrier = {};
		imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
		imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
		imageBarrier.oldLayout = currentLayout;
		imageBarrier.newLayout = newLayout;

		VkImageAspectFlags aspectMask = (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		imageBarrier.subresourceRange = ImageSubresourceRange(aspectMask);
		imageBarrier.image = image;

		VkDependencyInfo depInfo{};
		depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		depInfo.pNext = nullptr;

		depInfo.imageMemoryBarrierCount = 1;
		depInfo.pImageMemoryBarriers = &imageBarrier;

		vkCmdPipelineBarrier2(cmdBuffer, &depInfo);
	}

	VkImageSubresourceRange rVk::ImageSubresourceRange(VkImageAspectFlags aspectMask)
	{
		VkImageSubresourceRange subImage = {};
		subImage.aspectMask = aspectMask;
		subImage.baseMipLevel = 0;
		subImage.levelCount = VK_REMAINING_MIP_LEVELS;
		subImage.baseArrayLayer = 0;
		subImage.layerCount = VK_REMAINING_ARRAY_LAYERS;

		return subImage;
	}

	VkCommandBufferSubmitInfo rVk::CommandBufferSubmitInfo(VkCommandBuffer cmdBuffer)
	{
		VkCommandBufferSubmitInfo cmdBufferSubmitInfo = {};
		cmdBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		cmdBufferSubmitInfo.commandBuffer = cmdBuffer;
		cmdBufferSubmitInfo.deviceMask = 0;

		return cmdBufferSubmitInfo;
	}

	VkSemaphoreSubmitInfo rVk::SemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore)
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

	VkSubmitInfo2 rVk::SubmitInfo(VkCommandBufferSubmitInfo* cmdBufferInfo, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo)
	{
		VkSubmitInfo2 submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		submitInfo.waitSemaphoreInfoCount = 1;
		submitInfo.pWaitSemaphoreInfos = waitSemaphoreInfo;
		submitInfo.commandBufferInfoCount = 1;
		submitInfo.pCommandBufferInfos = cmdBufferInfo;
		submitInfo.signalSemaphoreInfoCount = 1;
		submitInfo.pSignalSemaphoreInfos = signalSemaphoreInfo;

		return submitInfo;
	}

	VkImageCreateInfo rVk::ImageCreateInfo(VkFormat format, VkImageUsageFlags imageUsageFlags, VkExtent3D extent)
	{
		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.pNext = nullptr;
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

	VkImageViewCreateInfo rVk::ImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags)
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

	void rVk::CopyImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize)
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
}