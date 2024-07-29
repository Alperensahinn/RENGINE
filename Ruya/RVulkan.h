#pragma once

#include <vector>

#include <vulkan/vulkan.h>

class RVulkan
{
public:
    RVulkan();
    ~RVulkan();

    RVulkan(const RVulkan&) = delete;
    RVulkan& operator=(const RVulkan&) = delete;

    RVulkan(RVulkan&& other) noexcept;
    RVulkan& operator=(RVulkan&& other) noexcept;

public:
	void Init();

	void CleanUp();

private:
	void CreateInstance();
    bool CheckValidationLayerSupport();
    void CreateDebugMessenger();
    void DestroyDebugUtilsMessenger();

private:
	VkInstance pInstance;
    std::vector<const char*> instanceExtensions = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
    std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
    VkDebugUtilsMessengerEXT pDebugUtilsMessanger;

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
};