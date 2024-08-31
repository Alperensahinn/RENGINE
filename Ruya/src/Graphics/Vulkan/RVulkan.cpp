#include "RVulkan.h"
#include "../../Utilities/FileSystem/FileSystem.h"
#include "../../Utilities/Log/RLog.h"
#include "../../EngineUI/EngineUI.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cmath>

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
		rvkCreateSwapChain(this);
		rvkSetQueues(this);
		rvkCreateDescriptors(this);
		CreateTrianglePipeline();
		rvkCreateEngineUIDescriptorPool(this);
		rvkCreateCommandPool(this);
		rvkCreateSynchronizationObjects(this);
	}

	void RVulkan::WaitDeviceForCleanUp()
	{
		vkDeviceWaitIdle(pDevice);
	}

	void RVulkan::CleanUp()
	{
		deletionQueue.flush();

		for (int i = 0; i < frameOverlap; i++)
		{
			vkDestroyCommandPool(pDevice, frames[i].commandPool, nullptr);
			vkDestroySemaphore(pDevice, frames[i].swapchainSemaphore, nullptr);
			vkDestroySemaphore(pDevice, frames[i].renderSemaphore, nullptr);
			vkDestroyFence(pDevice, frames[i].renderFence, nullptr);
		}

		for (int i = 0; i < swapChainImageViews.size(); i++)
		{
			vkDestroyImageView(pDevice, swapChainImageViews[i], nullptr);
		}

		vkDestroySwapchainKHR(pDevice, pSwapChain, nullptr);
		vkDestroyDevice(pDevice, nullptr);

		if (enableValidationLayers)
		{
			rvkDestroyDebugUtilsMessenger(this);
		}

		vkDestroySurfaceKHR(pInstance, pSurface, nullptr);
		vkDestroyInstance(pInstance, nullptr);
	}

	void RVulkan::Draw(EngineUI* pEngineUI, RVkMeshBuffer geometry, math::mat4 viewMatrix)
	{
		CHECK_VKRESULT_DEBUG(vkWaitForFences(pDevice, 1, &GetCurrentFrame().renderFence, VK_TRUE, UINT64_MAX));
		CHECK_VKRESULT_DEBUG(vkResetFences(pDevice, 1, &GetCurrentFrame().renderFence));

		uint32_t imageIndex;
		VkResult nextImageResult = vkAcquireNextImageKHR(pDevice, pSwapChain, UINT64_MAX, GetCurrentFrame().swapchainSemaphore, nullptr, &imageIndex);

		if(nextImageResult == VK_ERROR_OUT_OF_DATE_KHR)
		{
			resizeRequest = true;
			return;
		}

		VkCommandBuffer cmdBuffer = GetCurrentFrame().mainCommandBuffer;
		CHECK_VKRESULT_DEBUG(vkResetCommandBuffer(cmdBuffer, 0));

		VkCommandBufferBeginInfo cmdBufferbeginInfo = rvkCommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		drawExtent.width = drawImage.imageExtent.width;
		drawExtent.height = drawImage.imageExtent.height;

		CHECK_VKRESULT_DEBUG(vkBeginCommandBuffer(cmdBuffer, &cmdBufferbeginInfo));

		rvkTransitionImage(cmdBuffer, drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

		VkClearColorValue clearValue = { { 0.0f, 0.0f, 0.0f, 1.0f } };
		VkImageSubresourceRange clearRange = rvkImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

		vkCmdClearColorImage(cmdBuffer, drawImage.image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

		rvkTransitionImage(cmdBuffer, drawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		rvkTransitionImage(cmdBuffer, depthImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

		DrawGeometry(cmdBuffer, geometry, viewMatrix);

		DrawEngineUI(pEngineUI, cmdBuffer, drawImage.imageView);

		rvkTransitionImage(cmdBuffer, drawImage.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		rvkTransitionImage(cmdBuffer, swapChainImages[imageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		rvkCopyImageToImage(cmdBuffer, drawImage.image, swapChainImages[imageIndex], drawExtent, swapChainExtent);

		rvkTransitionImage(cmdBuffer, swapChainImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		CHECK_VKRESULT_DEBUG(vkEndCommandBuffer(cmdBuffer));

		VkCommandBufferSubmitInfo cmdBufferSubmitInfo = rvkCommandBufferSubmitInfo(cmdBuffer);
		VkSemaphoreSubmitInfo waitSempSubmitInfo = rvkSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, GetCurrentFrame().swapchainSemaphore);
		VkSemaphoreSubmitInfo signalSempSubmitInfo = rvkSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, GetCurrentFrame().renderSemaphore);
		VkSubmitInfo2 submitInfo = rvkSubmitInfo(&cmdBufferSubmitInfo, &signalSempSubmitInfo, &waitSempSubmitInfo);

		CHECK_VKRESULT_DEBUG(vkQueueSubmit2(pGraphicsQueue, 1, &submitInfo, GetCurrentFrame().renderFence));

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.pSwapchains = &pSwapChain;
		presentInfo.swapchainCount = 1;

		presentInfo.pWaitSemaphores = &GetCurrentFrame().renderSemaphore;
		presentInfo.waitSemaphoreCount = 1;

		presentInfo.pImageIndices = &imageIndex;

		VkResult queuePresentResult = vkQueuePresentKHR(pGraphicsQueue, &presentInfo);

		if (queuePresentResult == VK_ERROR_OUT_OF_DATE_KHR)
		{
			resizeRequest = true;
		}

		frameNumber++;

		GetCurrentFrame().deletionQueue.flush();
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

	void RVulkan::DrawEngineUI(EngineUI* pEngineUI, VkCommandBuffer cmd, VkImageView targetImageView)
	{
		VkRenderingAttachmentInfo colorAttachment = rvkCreateAttachmentInfo(targetImageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		VkRenderingInfo renderInfo = rvkCreateRenderingInfo(swapChainExtent, &colorAttachment, nullptr);

		vkCmdBeginRendering(cmd, &renderInfo);

		pEngineUI->DrawData(cmd);

		vkCmdEndRendering(cmd);
	}

	void RVulkan::DrawGeometry(VkCommandBuffer cmdBuffer, RVkMeshBuffer geometry, math::mat4 viewMatrix)
	{
		VkRenderingAttachmentInfo colorAttachment = rvkCreateAttachmentInfo(drawImage.imageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		VkRenderingAttachmentInfo depthAttachment = rvkDepthAttachmentInfo(depthImage.imageView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

		VkRenderingInfo renderInfo = rvkCreateRenderingInfo(drawExtent, &colorAttachment, &depthAttachment);
		vkCmdBeginRendering(cmdBuffer, &renderInfo);

		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipeline);

		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = drawExtent.width;
		viewport.height = drawExtent.height;
		viewport.minDepth = 1.f;
		viewport.maxDepth = 0.f;

		vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

		VkRect2D scissor = {};
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		scissor.extent.width = drawExtent.width;
		scissor.extent.height = drawExtent.height;

		vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

		glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)1600 / (float)900, 0.1f, 100.0f);
		glm::mat4 model = glm::mat4(1.0f);
		proj[1][1] *= -1;

		RVkDrawPushConstants push_constants;
		push_constants.worldMatrix = proj * viewMatrix * model;
		push_constants.vertexBuffer = geometry.vertexBufferAddress;

		vkCmdPushConstants(cmdBuffer, trianglePipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(RVkDrawPushConstants), &push_constants);
		vkCmdBindIndexBuffer(cmdBuffer, geometry.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(cmdBuffer, geometry.indexCount, 1, 0, 0, 0);

		vkCmdEndRendering(cmdBuffer);
	}

	void RVulkan::CreateTrianglePipeline()
	{
		VkShaderModule vertexShader;
		std::vector<char> colorTriangleVertexCode = Ruya::ReadBinaryFile("src/Graphics/Shaders/ColoredTriangleVertexShader.spv");
		vertexShader = rvkCreateShaderModule(this, colorTriangleVertexCode);

		VkShaderModule fragmentShader;
		std::vector<char> colorTriangleFragmentCode = Ruya::ReadBinaryFile("src/Graphics/Shaders/ColoredTriangleFragmentShader.spv");
		fragmentShader = rvkCreateShaderModule(this, colorTriangleFragmentCode);

		VkPushConstantRange pushConstantBufferRange = {};
		pushConstantBufferRange.offset = 0;
		pushConstantBufferRange.size = sizeof(RVkDrawPushConstants);
		pushConstantBufferRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantBufferRange;
		pipelineLayoutCreateInfo.pushConstantRangeCount = 1;

		CHECK_VKRESULT(vkCreatePipelineLayout(pDevice, &pipelineLayoutCreateInfo, nullptr, &trianglePipelineLayout));

		RVkPipelineBuilder pipelineBuilder;
		pipelineBuilder.Clear();
		pipelineBuilder.SetShaders(vertexShader, fragmentShader);
		pipelineBuilder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		pipelineBuilder.SetPolygonMode(VK_POLYGON_MODE_FILL);
		pipelineBuilder.SetCullMode(VK_CULL_MODE_FRONT_BIT, VK_FRONT_FACE_CLOCKWISE);
		pipelineBuilder.SetMultisampling(false);
		pipelineBuilder.SetBlending(false);
		pipelineBuilder.SetDepthTest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);
		pipelineBuilder.SetColorAttachmentFormat(drawImage.imageFormat);
		pipelineBuilder.SetDepthFormat(depthImage.imageFormat);
		pipelineBuilder.pipelineLayout = trianglePipelineLayout;

		trianglePipeline = pipelineBuilder.BuildPipeline(this);

		vkDestroyShaderModule(pDevice, vertexShader, nullptr);
		vkDestroyShaderModule(pDevice, fragmentShader, nullptr);

		deletionQueue.PushFunction([=]()
			{
				vkDestroyPipelineLayout(pDevice, trianglePipelineLayout, nullptr);
				vkDestroyPipeline(pDevice, trianglePipeline, nullptr);
			});
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

	void rvkCheckQueueFamilies(RVulkan* pRVulkan)
	{
		unsigned int queueFamilyPropertyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(pRVulkan->pPhysicalDevice, &queueFamilyPropertyCount, nullptr);

		if (queueFamilyPropertyCount == 0)
		{
			RERRLOG("[VULKAN ERROR] No queue families supported.")
		}

		std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(pRVulkan->pPhysicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());

		for (uint32_t i = 0; i < queueFamilyPropertyCount; i++)
		{
			if (queueFamilyProperties.at(i).queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				pRVulkan->graphicsQueueIndex = i;
			}
		}

		RLOG("[VULKAN] Queue family with graphics bit found. Index: " << pRVulkan->graphicsQueueIndex)
	}

	void rvkCreateDevice(RVulkan* pRVulkan)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = pRVulkan->graphicsQueueIndex;
		queueCreateInfo.queueCount = 1;

		float queuePrioritie = 1.0f;
		queueCreateInfo.pQueuePriorities = &queuePrioritie;
		
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
		deviceCreateInfo.queueCreateInfoCount = 1;
		deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
		deviceCreateInfo.pEnabledFeatures = nullptr;
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(pRVulkan->deviceExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = pRVulkan->deviceExtensions.data();

		CHECK_VKRESULT(vkCreateDevice(pRVulkan->pPhysicalDevice, &deviceCreateInfo, nullptr, &(pRVulkan->pDevice)));

		RLOG("[VULKAN] Logical device created.");
	}

	void rvkSetQueues(RVulkan* pRVulkan)
	{
		vkGetDeviceQueue(pRVulkan->pDevice, pRVulkan->graphicsQueueIndex, 0, &(pRVulkan->pGraphicsQueue));
		vkGetDeviceQueue(pRVulkan->pDevice, pRVulkan->graphicsQueueIndex, 0, &(pRVulkan->pPresentQueue));

		RLOG("[VULKAN] Device queues setted.");
	}

	void rvkCreateWindowSurface(RVulkan* pRVulkan)
	{
		VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.hwnd = glfwGetWin32Window(&(pRVulkan->window));
		surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);

		CHECK_VKRESULT(vkCreateWin32SurfaceKHR(pRVulkan->pInstance, &surfaceCreateInfo, nullptr, &(pRVulkan->pSurface)));

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

		VkImageCreateInfo renderImageCreateInfo = rvkImageCreateInfo(pRVulkan->drawImage.imageFormat, drawImageUsageFlags, pRVulkan->drawImage.imageExtent);

		VmaAllocationCreateInfo rimgAllocCreateInfo = {};
		rimgAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		rimgAllocCreateInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		CHECK_VKRESULT(vmaCreateImage(pRVulkan->vmaAllocator, &renderImageCreateInfo, &rimgAllocCreateInfo, &(pRVulkan->drawImage.image), &(pRVulkan->drawImage.allocation), nullptr));

		VkImageViewCreateInfo rimgCreateInfo = rvkImageViewCreateInfo(pRVulkan->drawImage.imageFormat, pRVulkan->drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

		CHECK_VKRESULT(vkCreateImageView(pRVulkan->pDevice, &rimgCreateInfo, nullptr, &(pRVulkan->drawImage.imageView)));

		pRVulkan->depthImage.imageFormat = VK_FORMAT_D32_SFLOAT;
		pRVulkan->depthImage.imageExtent = drawImageExtent;
		VkImageUsageFlags depthImageUsages = {};
		depthImageUsages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		VkImageCreateInfo dimg_info = rvkImageCreateInfo(pRVulkan->depthImage.imageFormat, depthImageUsages, drawImageExtent);

		CHECK_VKRESULT(vmaCreateImage(pRVulkan->vmaAllocator, &dimg_info, &rimgAllocCreateInfo, &(pRVulkan->depthImage.image), &(pRVulkan->depthImage.allocation), nullptr));

		VkImageViewCreateInfo dview_info = rvkImageViewCreateInfo(pRVulkan->depthImage.imageFormat, pRVulkan->depthImage.image, VK_IMAGE_ASPECT_DEPTH_BIT);

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

		pRVulkan->deletionQueue.PushFunction([vmaAllocator = pRVulkan->vmaAllocator]() {vmaDestroyAllocator(vmaAllocator);});

		RLOG("[VULKAN] Vulkan memory allocator created.")
	}

	void rvkCreateBuffer()
	{
		VkBufferCreateInfo bufferCreateInfo = {};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;


		//vmaCreateBuffer(vmaAllocator, );
	}

	void rvkCreateDescriptors(RVulkan* pRVulkan)
	{
		std::vector<RVkDescriptorAllocator::PoolSizeRatio> sizes =
		{
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
		};

		pRVulkan->globalDescriptorAllocator.InitPool(pRVulkan, 10, sizes);

		{
			RVkDescriptorLayoutBuilder builder;
			builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
			pRVulkan->drawImageDescriptorLayout = builder.Build(pRVulkan, VK_SHADER_STAGE_COMPUTE_BIT);
		}

		pRVulkan->drawImageDescriptors = pRVulkan->globalDescriptorAllocator.Allocate(pRVulkan, pRVulkan->drawImageDescriptorLayout);

		VkDescriptorImageInfo dscImageInfo = {};
		dscImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		dscImageInfo.imageView = pRVulkan->drawImage.imageView;

		VkWriteDescriptorSet writeDescriptorSet = {};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = pRVulkan->drawImageDescriptors;
		writeDescriptorSet.dstBinding = 0;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		writeDescriptorSet.pImageInfo = &dscImageInfo;

		vkUpdateDescriptorSets(pRVulkan->pDevice, 1, &writeDescriptorSet, 0, nullptr);

		pRVulkan->deletionQueue.PushFunction([=]()
			{
				pRVulkan->globalDescriptorAllocator.DestroyPool(pRVulkan);
				vkDestroyDescriptorSetLayout(pRVulkan->pDevice, pRVulkan->drawImageDescriptorLayout, nullptr);

			});

		RLOG("[VULKAN] Descriptor sets created.");
	}

	VkCommandBufferBeginInfo rvkCommandBufferBeginInfo(VkCommandBufferUsageFlags flags)
	{
		VkCommandBufferBeginInfo commandBufferBeginInfo = {};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.flags = flags;
		return commandBufferBeginInfo;
	}


	void rvkTransitionImage(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout)
	{
		VkImageMemoryBarrier2 imageBarrier = {};
		imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
		imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
		imageBarrier.oldLayout = currentLayout;
		imageBarrier.newLayout = newLayout;

		VkImageAspectFlags aspectMask = {};
		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
		{
			aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		}

		else
		{
			aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}
		imageBarrier.subresourceRange = rvkImageSubresourceRange(aspectMask);
		imageBarrier.image = image;

		VkDependencyInfo depInfo{};
		depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		depInfo.pNext = nullptr;

		depInfo.imageMemoryBarrierCount = 1;
		depInfo.pImageMemoryBarriers = &imageBarrier;

		vkCmdPipelineBarrier2(cmdBuffer, &depInfo);
	}

	VkImageSubresourceRange rvkImageSubresourceRange(VkImageAspectFlags aspectMask)
	{
		VkImageSubresourceRange subImage = {};
		subImage.aspectMask = aspectMask;
		subImage.baseMipLevel = 0;
		subImage.levelCount = VK_REMAINING_MIP_LEVELS;
		subImage.baseArrayLayer = 0;
		subImage.layerCount = VK_REMAINING_ARRAY_LAYERS;

		return subImage;
	}

	VkCommandBufferSubmitInfo rvkCommandBufferSubmitInfo(VkCommandBuffer cmdBuffer)
	{
		VkCommandBufferSubmitInfo cmdBufferSubmitInfo = {};
		cmdBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		cmdBufferSubmitInfo.commandBuffer = cmdBuffer;
		cmdBufferSubmitInfo.deviceMask = 0;

		return cmdBufferSubmitInfo;
	}

	VkSemaphoreSubmitInfo rvkSemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore)
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

	VkSubmitInfo2 rvkSubmitInfo(VkCommandBufferSubmitInfo* cmdBufferInfo, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo)
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

	VkImageCreateInfo rvkImageCreateInfo(VkFormat format, VkImageUsageFlags imageUsageFlags, VkExtent3D extent)
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

	VkImageViewCreateInfo rvkImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags)
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

	VkRenderingAttachmentInfo rvkCreateAttachmentInfo(VkImageView view, VkClearValue* clear, VkImageLayout layout)
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

	VkRenderingInfo rvkCreateRenderingInfo(VkExtent2D renderExtent, VkRenderingAttachmentInfo* colorAttachment, VkRenderingAttachmentInfo* depthAttachment)
	{
		VkRenderingInfo renderingInfo = {};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderingInfo.renderArea = VkRect2D{ VkOffset2D { 0, 0 }, renderExtent };
		renderingInfo.layerCount = 1;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachments = colorAttachment;
		renderingInfo.pDepthAttachment = depthAttachment;
		renderingInfo.pStencilAttachment = nullptr;

		return renderingInfo;
	}

	VkPipelineShaderStageCreateInfo rvkCreateShaderStageInfo(VkShaderModule shaderModule, VkShaderStageFlagBits shaderStageFlag)
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
		CHECK_VKRESULT(vmaCreateBuffer(pRulkan->vmaAllocator, &bufferCreateInfo, &allocCreateInfo, &buffer.buffer, &buffer.allocation, &buffer.allocationInfo));

		return buffer;
	}

	void rvkDestoryBuffer(RVulkan* pRulkan, RVkAllocatedBuffer& buffer)
	{
		vmaDestroyBuffer(pRulkan->vmaAllocator, buffer.buffer, buffer.allocation);
	}

	RVkMeshBuffer rvkLoadMesh(RVulkan* pRVulkan, std::vector<Vertex> vertices, std::vector<uint32_t> indices)
	{
		const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
		const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

		RVkMeshBuffer meshBuffer;
		meshBuffer.indexCount = indices.size();

		meshBuffer.vertexBuffer = rvkCreateBuffer(pRVulkan, vertexBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

		VkBufferDeviceAddressInfo deviceAdressInfo{ .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,.buffer = meshBuffer.vertexBuffer.buffer };
		meshBuffer.vertexBufferAddress = vkGetBufferDeviceAddress(pRVulkan->pDevice, &deviceAdressInfo);

		meshBuffer.indexBuffer = rvkCreateBuffer(pRVulkan, indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY);

		RVkAllocatedBuffer staging = rvkCreateBuffer(pRVulkan, vertexBufferSize + indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

		void* data;
		vmaMapMemory(pRVulkan->vmaAllocator, staging.allocation, &data);

		memcpy(data, vertices.data(), vertexBufferSize);
		memcpy((char*)data + vertexBufferSize, indices.data(), indexBufferSize);

		vmaUnmapMemory(pRVulkan->vmaAllocator, staging.allocation);

		rvkImmediateSubmit(pRVulkan, [&](VkCommandBuffer cmd)
			{
			VkBufferCopy vertexCopy{ 0 };
			vertexCopy.dstOffset = 0;
			vertexCopy.srcOffset = 0;
			vertexCopy.size = vertexBufferSize;

			vkCmdCopyBuffer(cmd, staging.buffer, meshBuffer.vertexBuffer.buffer, 1, &vertexCopy);

			VkBufferCopy indexCopy{ 0 };
			indexCopy.dstOffset = 0;
			indexCopy.srcOffset = vertexBufferSize;
			indexCopy.size = indexBufferSize;

			vkCmdCopyBuffer(cmd, staging.buffer, meshBuffer.indexBuffer.buffer, 1, &indexCopy);
			});

		rvkDestoryBuffer(pRVulkan, staging);

		return meshBuffer;
	}

	void rvkImmediateSubmit(RVulkan* pRVulkan, std::function<void(VkCommandBuffer cmd)>&& function)
	{
		CHECK_VKRESULT(vkResetFences(pRVulkan->pDevice, 1, &(pRVulkan->immediateFence)));
		CHECK_VKRESULT(vkResetCommandBuffer(pRVulkan->immediateCommandBuffer, 0));

		VkCommandBuffer cmdBuffer = pRVulkan->immediateCommandBuffer;

		VkCommandBufferBeginInfo cmdBeginInfo = rvkCommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		CHECK_VKRESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo));

		function(cmdBuffer);

		CHECK_VKRESULT(vkEndCommandBuffer(cmdBuffer));

		VkCommandBufferSubmitInfo cmdinfo = rvkCommandBufferSubmitInfo(cmdBuffer);
		VkSubmitInfo2 submit = rvkSubmitInfo(&cmdinfo, nullptr, nullptr);

		CHECK_VKRESULT(vkQueueSubmit2(pRVulkan->pGraphicsQueue, 1, &submit, pRVulkan->immediateFence));

		CHECK_VKRESULT(vkWaitForFences(pRVulkan->pDevice, 1, &(pRVulkan->immediateFence), true, UINT32_MAX));
	}

	VkRenderingAttachmentInfo  rvkDepthAttachmentInfo(VkImageView view, VkImageLayout layout)
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

	void RVkDescriptorAllocator::InitPool(RVulkan* pRVulkan, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios)
	{
		std::vector<VkDescriptorPoolSize> poolSizes;
		for (int i = 0; i < poolRatios.size(); i++)
		{
			VkDescriptorPoolSize dscPoolSize = {};
			dscPoolSize.type = poolRatios[i].descriptorType;
			dscPoolSize.descriptorCount = uint32_t(poolRatios[i].ratio * maxSets);
			
			poolSizes.push_back(dscPoolSize);
		}

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
		descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCreateInfo.flags = 0;
		descriptorPoolCreateInfo.maxSets = maxSets;
		descriptorPoolCreateInfo.poolSizeCount = (uint32_t)poolSizes.size();
		descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();
		
		CHECK_VKRESULT(vkCreateDescriptorPool(pRVulkan->pDevice, &descriptorPoolCreateInfo, nullptr, &descriptorPool));
	}

	void RVkDescriptorAllocator::ClearDescriptors(RVulkan* pRVulkan)
	{
		CHECK_VKRESULT(vkResetDescriptorPool(pRVulkan->pDevice, descriptorPool, 0));
	}

	void RVkDescriptorAllocator::DestroyPool(RVulkan* pRVulkan)
	{
		vkDestroyDescriptorPool(pRVulkan->pDevice, descriptorPool, nullptr);
	}

	VkDescriptorSet RVkDescriptorAllocator::Allocate(RVulkan* pRVulkan, VkDescriptorSetLayout layout)
	{
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.descriptorPool = descriptorPool;
		descriptorSetAllocateInfo.descriptorSetCount = 1;
		descriptorSetAllocateInfo.pSetLayouts = &layout;

		VkDescriptorSet descriptorSet;
		CHECK_VKRESULT(vkAllocateDescriptorSets(pRVulkan->pDevice, &descriptorSetAllocateInfo, &descriptorSet));

		return descriptorSet;
	}

	RVkPipelineBuilder::RVkPipelineBuilder()
	{

	}

	RVkPipelineBuilder::~RVkPipelineBuilder()
	{
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
		colorBlendStateCreateInfo.attachmentCount = 1;
		colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;

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
		colorBlendAttachmentState = {};
		multisamplingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		pipelineLayout = {};
		depthStencilCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
		pipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
	}

	void RVkPipelineBuilder::SetShaders(VkShaderModule vertexShader, VkShaderModule fragmentShader)
	{
		shaderStages.clear();
		shaderStages.push_back(rvkCreateShaderStageInfo(vertexShader, VK_SHADER_STAGE_VERTEX_BIT));
		shaderStages.push_back(rvkCreateShaderStageInfo(fragmentShader, VK_SHADER_STAGE_FRAGMENT_BIT));
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
			colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachmentState.blendEnable = VK_FALSE;
		}
	}

	void RVkPipelineBuilder::SetColorAttachmentFormat(VkFormat format)
	{
		colorAttachmentformat = format;
		pipelineRenderingCreateInfo.colorAttachmentCount = 1;
		pipelineRenderingCreateInfo.pColorAttachmentFormats = &colorAttachmentformat;
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
}