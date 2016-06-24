#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW\glfw3.h>
#include <vulkan\vulkan.h>
#include <cstdlib>
#include <assert.h>
#include <vector>

#include "DataStructures.h"

using namespace DataStructures;

namespace VulkanResources
{
	class VKResources
	{
	public:
		VKResources();
		~VKResources();

		void InitVKInstance();
		void InitDevice();
		void CreateSurface( GLFWwindow * window );
		void CreateSwapChain( int windowHeight, int windowWidth );
		void CreateCommandPool();
		void PresentQueue( uint32_t imageIndex );

		uint32_t AcquireImage();

		VkInstance GetInstance();
		VkSurfaceKHR GetSurface();

		void DestroyVKInstance();
		void DestroySwapChain();
		void DestroyDevice();
		void DestroyCommandPool();
		void DestroySurface();

	private:
		void CreateSemaphores();
		void CreateCommandBuffers();
		void RecordCommandBuffers();
		bool CheckAndSelectGPU( std::vector<VkPhysicalDevice> &gpuList );

		VkSurfaceFormatKHR GetSwapChainFormat( std::vector<VkSurfaceFormatKHR> &surfaceFormats );

		// Instance and devices
		VkInstance						vkInstance					= VK_NULL_HANDLE;
		VkDevice						vkDevice					= VK_NULL_HANDLE;
		VkPhysicalDevice				selectedGPU					= VK_NULL_HANDLE;
		VkPhysicalDeviceProperties		GPUProperties				= {};
		VkPhysicalDeviceFeatures		GPUFeatures					= {};

		// Swap chain
		VkSwapchainKHR					vkSwapChain					= VK_NULL_HANDLE;
		std::vector<SwapChainBuffer>	swapChainBuffers;

		// Command pool
		VkCommandPool					vkCommandPool				= VK_NULL_HANDLE;
		VkQueue							vkGraphicsQueue				= VK_NULL_HANDLE;
		VkQueue							vkPresentQueue				= VK_NULL_HANDLE;
		VkFence							vkFence						= VK_NULL_HANDLE;
		VkSemaphore						imageAvailableSemaphore		= VK_NULL_HANDLE;
		VkSemaphore						renderingFinishedSemaphore	= VK_NULL_HANDLE;
		uint32_t						graphicsQueueFamilyIndex	= UINT32_MAX;
		uint32_t						presentQueueFamilyIndex		= UINT32_MAX;
		uint32_t						imageCount					= 0;
		std::vector<VkCommandBuffer>	vkCommandBuffers;

		VkSurfaceKHR					vkSurface					= VK_NULL_HANDLE;
		uint32_t						surfaceHeight				= 600;
		uint32_t						surfaceWidth				= 940;
	};
}
