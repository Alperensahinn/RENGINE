#pragma once

#define RVULKAN

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#endif

#include "vk_mem_alloc.h"

#include <cassert>
#include <iostream>

#define CHECK_VKRESULT(exp)                                      \
    {                                                            \
        VkResult result = (exp);                                 \
        if (VK_SUCCESS != result)                                \
        {                                                        \
            std::cout << "[VULKAN ERROR] " << #exp               \
                      << ": Failed with VkResult: " << result    \
                      << std::endl;                              \
            assert(false);                                       \
        }                                                        \
    }

#define CHECK_VKRESULT_DEBUG(exp)                                  \
    {                                                              \
        VkResult result = (exp);                                   \
        if (VK_SUCCESS != result)                                  \
        {                                                          \
            std::cout << "[VULKAN ERROR] " << #exp                 \
                      << ": Failed with VkResult: " << result      \
                      << std::endl;                                \
            assert(false);                                         \
        }                                                          \
    }

#define VK_VALIDATION_LOG(exp)                                     \
    {                                                              \
            std::cout << "[VULKAN VALIDATION LAYER] "        \
                      << exp << "\n" << std::endl;                         \
    }

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        VK_VALIDATION_LOG(pCallbackData->pMessage)
    }

    return VK_FALSE;
}

#ifdef NDEBUG
#define CHECK_VKRESULT_DEBUG(exp)
#endif
