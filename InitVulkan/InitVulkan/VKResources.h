#pragma once

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
		void CheckAndSelectGPU( std::vector<VkPhysicalDevice> &gpuList );
		void CreateCommandPool();
		void CreateSwapChain();
		void GetSwapChainNext( VkSemaphore presentCompleteSemaphore, uint32_t imageIndex );

		VkInstance GetInstance();
		VkSurfaceKHR GetSurface();

		void DestroyVKInstance();
		void DestroySwapChain();
		void DestroyDevice();
		void DestroyCommandPool();
		void DestroySurface();

	private:
		void CreateCommandBuffers();
		void RecordCommandBuffers();

		VkInstance						vkInstance				= VK_NULL_HANDLE;
		VkDevice						vkDevice				= VK_NULL_HANDLE;
		VkPhysicalDevice				gpu						= VK_NULL_HANDLE;
		VkPhysicalDeviceProperties		gpuProperties			= {};

		// Swap chain
		VkSwapchainKHR					vkSwapChain				= VK_NULL_HANDLE;
		std::vector<SwapChainBuffer>	swapChainBuffers;

		// Command pool
		VkCommandPool					vkCommandPool			= VK_NULL_HANDLE;
		VkQueue							vkQueue					= VK_NULL_HANDLE;
		VkFence							vkFence					= VK_NULL_HANDLE;
		VkSemaphore						vkSemaphore				= VK_NULL_HANDLE;
		uint32_t						graphicsFamilyIndex		= 0;
		uint32_t						imageCount				= 0;
		std::vector<VkCommandBuffer>	vkCommandBuffers;

		VkSurfaceKHR					vkSurface				= VK_NULL_HANDLE;
		int								windowHeight			= 640;
		int								windowWidth				= 960;
		uint32_t						surfaceHeight			= 600;
		uint32_t						surfaceWidth			= 940;

		//GLFWwindow*						window					= VK_NULL_HANDLE;
	};
}
