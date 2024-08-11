#pragma once

#include <vector>

#if defined(_WIN32)
    #define VK_USE_PLATFORM_WIN32_KHR
    #include <vulkan/vulkan.h>
#endif



struct GLFWwindow;

class RVulkan
{
public:
    RVulkan(GLFWwindow& window);
    ~RVulkan();

    RVulkan(const RVulkan&) = delete;
    RVulkan& operator=(const RVulkan&) = delete;

    RVulkan(RVulkan&& other) noexcept;
    RVulkan& operator=(RVulkan&& other) noexcept;

public:
	void Init(GLFWwindow& window);

	void CleanUp();

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
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
};