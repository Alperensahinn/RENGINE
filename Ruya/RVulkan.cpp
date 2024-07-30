#include "RVulkan.h"

#include <iostream>
#include <stdexcept>

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
		std::cerr << "[VULKAN VAlIDATION LAYER] " << pCallbackData->pMessage << std::endl;
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

RVulkan::RVulkan(RVulkan&& other) noexcept
	: pInstance(other.pInstance),
	pDebugUtilsMessanger(other.pDebugUtilsMessanger),
	pPhysicalDevice(other.pPhysicalDevice),
	graphicsQueueIndex(other.graphicsQueueIndex),
	pDevice(other.pDevice),
	pSurface(other.pSurface)
{
	other.pInstance = VK_NULL_HANDLE;
	other.pDebugUtilsMessanger = VK_NULL_HANDLE;
	other.pPhysicalDevice = VK_NULL_HANDLE;
	graphicsQueueIndex = NULL;
	other.pDevice = VK_NULL_HANDLE;
	other.pSurface = VK_NULL_HANDLE;
}

RVulkan& RVulkan::operator=(RVulkan&& other) noexcept
{
	if (this != &other)
	{
		if (pInstance != VK_NULL_HANDLE)
		{
			vkDestroyInstance(pInstance, nullptr);
		}

		pInstance = other.pInstance;
		pDebugUtilsMessanger = other.pDebugUtilsMessanger;
		pPhysicalDevice = other.pPhysicalDevice;
		graphicsQueueIndex = other.graphicsQueueIndex;
		pDevice = other.pDevice;
		pSurface = other.pSurface;

		other.pInstance = VK_NULL_HANDLE;
		other.pDebugUtilsMessanger = VK_NULL_HANDLE;
		other.pPhysicalDevice = VK_NULL_HANDLE;
		other.graphicsQueueIndex = NULL;
		other.pDevice = VK_NULL_HANDLE;
		other.pSurface = VK_NULL_HANDLE;
	}
	return *this;
}

void RVulkan::Init(GLFWwindow& window)
{
	CreateInstance();
	CreateDebugMessenger();
	CreateWindowSurface(window);
	SelectPhysicalDevice();
	CheckQueueFamilies();
	CreateDevice();
	SetQueues();
}

void RVulkan::CleanUp()
{
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
		std::cerr << "[VULKAN ERROR] Validation layers requested, but not available." << std::endl;
		throw std::runtime_error("[VULKAN ERROR] Validation layers requested, but not available.");
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

		std::cout << "[VULKAN] Validation layers enabled." << std::endl;
	}
	else 
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	VkResult result = vkCreateInstance(&createInfo, nullptr, &pInstance);

	if (result != VK_SUCCESS)
	{
		std::cerr << "[VULKAN ERROR] Failed to create Vulkan instance." << std::endl;
		throw std::runtime_error("[VULKAN ERROR] Failed to create Vulkan instance.");
		return;
	}

	std::cout << "[VULKAN] Vulkan instance created." << std::endl;
}

bool RVulkan::CheckValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

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
			std::cerr << "[VULKAN ERROR] Failed to set up debug messenger." << std::endl;
			throw std::runtime_error("[VULKAN ERROR] Failed to set up debug messenger.");
		}
	}
	else 
	{
		std::cerr << "[VULKAN ERROR] Failed to find vkCreateDebugUtilsMessengerEXT." << std::endl;
		throw std::runtime_error("[VULKAN ERROR] Failed to find vkCreateDebugUtilsMessengerEXT.");
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
	VkResult result = vkEnumeratePhysicalDevices(pInstance, &physicalDevicesCount, nullptr);

	if (physicalDevicesCount == 0)
	{
		std::cerr << "[VULKAN ERROR] Failed to find Vulkan physical device." << std::endl;
		throw std::runtime_error("[VULKAN ERROR] Failed to find Vulkan physical device.");
		return;
	}

	else if(result != VK_SUCCESS)
	{
		std::cerr << "[VULKAN ERROR] Failed to enumerate physical devices." << std::endl;
		throw std::runtime_error("[VULKAN ERROR] Failed to enumerate physical devices.");
		return;
	}

	std::vector<VkPhysicalDevice> physicalDevices(physicalDevicesCount);
	result = vkEnumeratePhysicalDevices(pInstance, &physicalDevicesCount, physicalDevices.data());

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
	std::cout << "[VULKAN] Vulkan physical device selected: " << physicalDeviceProperties.deviceName << std::endl;
}

void RVulkan::CheckQueueFamilies()
{
	unsigned int queueFamilyPropertyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(pPhysicalDevice, &queueFamilyPropertyCount, nullptr);

	if(queueFamilyPropertyCount == 0)
	{
		std::cerr << "[VULKAN ERROR] No queue families supported." << std::endl;
		throw std::runtime_error("[VULKAN ERROR] No queue families supported.");
		return;
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

	std::cout << "[VULKAN] Queue family with graphics bit found. Index: " << graphicsQueueIndex << std::endl;
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

	VkResult result = vkCreateDevice(pPhysicalDevice, &deviceCreateInfo, nullptr, &pDevice);
	if(result != VK_SUCCESS)
	{
		std::cerr << "[VULKAN ERROR] Failed to create device." << std::endl;
		throw std::runtime_error("[VULKAN ERROR] Failed to create device.");
		return;
	}

	std::cout << "[VULKAN] Logical device created." << std::endl;
}

void RVulkan::SetQueues()
{
	vkGetDeviceQueue(pDevice, graphicsQueueIndex, 0, &pGraphicsQueue);
	vkGetDeviceQueue(pDevice, graphicsQueueIndex, 0, &pPresentQueue);

	std::cout << "[VULKAN] Device queues setted." << std::endl;
}

void RVulkan::CreateWindowSurface(GLFWwindow& window)
{
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hwnd = glfwGetWin32Window(&window);
	surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);

	VkResult result = vkCreateWin32SurfaceKHR(pInstance, &surfaceCreateInfo, nullptr, &pSurface);
	if (result != VK_SUCCESS)
	{
		std::cerr << "[VULKAN ERROR] Failed to create WIN32 surface." << std::endl;
		throw std::runtime_error("[VULKAN ERROR] Failed to create WIN32 surface.");
		return;
	}

	std::cout << "[VULKAN] Window surface created." << std::endl;
}
