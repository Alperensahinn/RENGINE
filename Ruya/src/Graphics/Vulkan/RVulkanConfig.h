#pragma once

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#endif

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

#ifdef NDEBUG
#define CHECK_VKRESULT_DEBUG(exp)
#endif
