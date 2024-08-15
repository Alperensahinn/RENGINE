#include "RVulkan.h"
#include "../../Utilities/FileSystem/FileSystem.h"
#include "../../Utilities/Log/RLog.h"

#include <iostream>
#include <stdexcept>
#include <algorithm>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) 
	{
		RERRLOG("[VULKAN VAlIDATION LAYER]" << pCallbackData->pMessage)
	}

	return VK_FALSE;
}

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
	CreateInstance();
	CreateDebugMessenger();
	CreateWindowSurface(window);
	SelectPhysicalDevice();
	CheckQueueFamilies();
	CreateDevice();
	CreateSwapChain(window);
	SetQueues();

	CreateGraphicsPipeline();
}

void RVulkan::CleanUp()
{
	for(int i = 0; i < swapChainImageViews.size(); i++)
	{
		if (swapChainImageViews[i] != VK_NULL_HANDLE)
		{
			vkDestroyImageView(pDevice, swapChainImageViews[i], nullptr);
			swapChainImageViews[i] = VK_NULL_HANDLE;
		}
	}

	if (pSwapChain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(pDevice, pSwapChain, nullptr);
		pSwapChain = VK_NULL_HANDLE;
	}

	if (pDevice != VK_NULL_HANDLE)
	{
		vkDestroyDevice(pDevice, nullptr);
		pDevice = VK_NULL_HANDLE;
	}

	if (enableValidationLayers) 
	{
		DestroyDebugUtilsMessenger();
	}

	if (pSurface != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR(pInstance, pSurface, nullptr);
		pSurface = VK_NULL_HANDLE;
	}

	if (pInstance != VK_NULL_HANDLE)
	{
		vkDestroyInstance(pInstance, nullptr);
		pInstance = VK_NULL_HANDLE;
	}
}

void RVulkan::CreateInstance()
{	
	VkApplicationInfo applicationInfo = {};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &applicationInfo;

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};


	if (enableValidationLayers && !CheckValidationLayerSupport())
	{
		RERRLOG("[VULKAN ERROR] Validation layers requested, but not available.")
	}

	if (enableValidationLayers && CheckValidationLayerSupport()) 
	{
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback = debugCallback;

		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
		createInfo.ppEnabledExtensionNames = instanceExtensions.data();

		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

		RLOG("[VULKAN] Validation layers enabled.");
	}
	else 
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	CHECK_VKRESULT(vkCreateInstance(&createInfo, nullptr, &pInstance));

	RLOG("[VULKAN] Vulkan instance created.")
}

bool RVulkan::CheckValidationLayerSupport()
{
	uint32_t layerCount;
	CHECK_VKRESULT(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));
	std::vector<VkLayerProperties> availableLayers(layerCount);
	CHECK_VKRESULT(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()));

	for (const char* layerName : validationLayers) 
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


void RVulkan::CreateDebugMessenger() 
{
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
	debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugCreateInfo.pfnUserCallback = debugCallback;
	
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(pInstance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		if(func(pInstance, &debugCreateInfo, nullptr, &pDebugUtilsMessanger) != VK_SUCCESS)
		{
			RERRLOG("[VULKAN ERROR] Failed to set up debug messenger.")
		}
	}
	else 
	{
		RERRLOG("[VULKAN ERROR] Failed to find vkCreateDebugUtilsMessengerEXT.")
	}
}

void RVulkan::DestroyDebugUtilsMessenger()
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(pInstance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(pInstance, pDebugUtilsMessanger, nullptr);
	}
}

void RVulkan::SelectPhysicalDevice()
{
	unsigned int physicalDevicesCount;
	CHECK_VKRESULT(vkEnumeratePhysicalDevices(pInstance, &physicalDevicesCount, nullptr));

	if (physicalDevicesCount == 0)
	{
		RERRLOG("[VULKAN ERROR] Failed to find Vulkan physical device.")
	}

	std::vector<VkPhysicalDevice> physicalDevices(physicalDevicesCount);
	CHECK_VKRESULT(vkEnumeratePhysicalDevices(pInstance, &physicalDevicesCount, physicalDevices.data()));

	for (VkPhysicalDevice physicalDevice : physicalDevices)
	{
		VkPhysicalDeviceProperties physicalDeviceProperties;

		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

		if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			pPhysicalDevice = physicalDevice;
			break;
		}
	}

	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(pPhysicalDevice, &physicalDeviceProperties);
	RLOG("[VULKAN] Vulkan physical device selected: " << physicalDeviceProperties.deviceName)
}

void RVulkan::CheckQueueFamilies()
{
	unsigned int queueFamilyPropertyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(pPhysicalDevice, &queueFamilyPropertyCount, nullptr);

	if(queueFamilyPropertyCount == 0)
	{
		RERRLOG("[VULKAN ERROR] No queue families supported.")
	}

	std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(pPhysicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());

	for(int i = 0; i < queueFamilyPropertyCount; i++)
	{
		if (queueFamilyProperties.at(i).queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphicsQueueIndex = i;
		}
	}

	RLOG("[VULKAN] Queue family with graphics bit found. Index: " << graphicsQueueIndex)
}

void RVulkan::CreateDevice()
{
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = graphicsQueueIndex;
	queueCreateInfo.queueCount = 1;

	float queuePrioritie = 1.0f;
	queueCreateInfo.pQueuePriorities = &queuePrioritie;

	VkPhysicalDeviceFeatures physicalDeviceFutures = {};

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceCreateInfo.pEnabledFeatures = &physicalDeviceFutures;
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

	CHECK_VKRESULT(vkCreateDevice(pPhysicalDevice, &deviceCreateInfo, nullptr, &pDevice));

	RLOG("[VULKAN] Logical device created.");
}

void RVulkan::SetQueues()
{
	vkGetDeviceQueue(pDevice, graphicsQueueIndex, 0, &pGraphicsQueue);
	vkGetDeviceQueue(pDevice, graphicsQueueIndex, 0, &pPresentQueue);

	RLOG("[VULKAN] Device queues setted.");
}

void RVulkan::CreateWindowSurface(GLFWwindow& window)
{
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hwnd = glfwGetWin32Window(&window);
	surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);

	CHECK_VKRESULT(vkCreateWin32SurfaceKHR(pInstance, &surfaceCreateInfo, nullptr, &pSurface));

	RLOG("[VULKAN] Window surface created.")
}

void RVulkan::CreateSwapChain(GLFWwindow& window)
{
	VkSurfaceCapabilitiesKHR capabilities;
	CHECK_VKRESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pPhysicalDevice, pSurface, &capabilities))

	std::vector<VkSurfaceFormatKHR> surfaceFormats;
	uint32_t surfaceFormatCount;

	CHECK_VKRESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(pPhysicalDevice, pSurface, &surfaceFormatCount, nullptr))

	if(surfaceFormatCount > 0)
	{
		surfaceFormats.resize(surfaceFormatCount);
		CHECK_VKRESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(pPhysicalDevice, pSurface, &surfaceFormatCount, surfaceFormats.data()))
	}
	
	std::vector<VkPresentModeKHR> presentModes;
	uint32_t presentModeCount;

	CHECK_VKRESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(pPhysicalDevice, pSurface, &presentModeCount, nullptr))

	if(presentModeCount > 0)
	{
		presentModes.resize(presentModeCount);
		CHECK_VKRESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(pPhysicalDevice, pSurface, &presentModeCount, presentModes.data()))
	}

	if(presentModes.empty() || surfaceFormats.empty())
	{
		std::cerr << "[VULKAN ERROR] Device not support swap chain." << std::endl;
		throw std::runtime_error("[VULKAN ERROR] Device not support swap chain.");
		return;
	}


	VkSurfaceFormatKHR surfaceFormat;
	bool bSRGBAvailable = false;

	for(VkSurfaceFormatKHR format : surfaceFormats)
	{
		if(format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && format.format == VK_FORMAT_B8G8R8A8_SRGB)
		{
			surfaceFormat = format;
			bSRGBAvailable = true;
		}
	}

	if(bSRGBAvailable == false)
	{
		surfaceFormat = surfaceFormats[0];
	}

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

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
	swapChainCreateInfo.surface = pSurface;
	swapChainCreateInfo.minImageCount = imageCount;
	swapChainCreateInfo.imageFormat = surfaceFormat.format;
	swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapChainCreateInfo.imageExtent = actualExtent;
	swapChainCreateInfo.imageArrayLayers = 1;
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t queueFamilyIndices[] = {0,0};

	if(pGraphicsQueue == pPresentQueue)
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

	CHECK_VKRESULT(vkCreateSwapchainKHR(pDevice, &swapChainCreateInfo, nullptr, &pSwapChain));

	uint32_t swapChainImageCount;
	CHECK_VKRESULT(vkGetSwapchainImagesKHR(pDevice, pSwapChain, &swapChainImageCount, nullptr));
	swapChainImages.resize(swapChainImageCount);
	swapChainImageViews.resize(swapChainImageCount);
	CHECK_VKRESULT(vkGetSwapchainImagesKHR(pDevice, pSwapChain, &swapChainImageCount, swapChainImages.data()));

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = actualExtent;

	for (int i = 0; i < swapChainImages.size(); i++) 
	{
		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = swapChainImages.data()[i];
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = swapChainImageFormat;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		CHECK_VKRESULT(vkCreateImageView(pDevice, &imageViewCreateInfo, nullptr, &swapChainImageViews.data()[i]));
	}

	RLOG("[VULKAN] Swap chain created.")
}

void RVulkan::CreateGraphicsPipeline()
{
	VkShaderModule vertexShaderModule;
	VkShaderModule fragmentShaderModule;

	CreateShaderModule(vertexShaderModule);
	CreateShaderModule(fragmentShaderModule);

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
	viewport.width = (float)swapChainExtent.width;
	viewport.height = (float)swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = &viewport;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors = &scissor;


	VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo = {};





	vkDestroyShaderModule(pDevice, vertexShaderModule, nullptr);
	vkDestroyShaderModule(pDevice, fragmentShaderModule, nullptr);


	RLOG("[VULKAN] Graphics pipeline created.")
}

void RVulkan::CreateShaderModule(VkShaderModule& shaderModule)
{
	std::vector<char> vertShaderCode = Ruya::ReadBinaryFile("src/Graphics/Shaders/vert.spv");
	std::vector<char> fragShaderCode = Ruya::ReadBinaryFile("src/Graphics/Shaders/frag.spv");

	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = vertShaderCode.size();
	shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(vertShaderCode.data());

	CHECK_VKRESULT(vkCreateShaderModule(pDevice, &shaderModuleCreateInfo, nullptr, &shaderModule));
}
