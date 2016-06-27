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
		void SubmitBuffers( uint32_t imageIndex, GLFWwindow* vkWindow );
		void CreateRenderPass();
		void CreateFrameBuffers();
		void OnWindowSizeChanged( GLFWwindow* vkWindow, int windowWidth, int windowHeight );

		uint32_t AcquireImageIndex( GLFWwindow* vkWindow );
		VkInstance GetInstance();
		VkSurfaceKHR GetSurface();

	private:
		void CreateSemaphores();
		void CreateCommandBuffers();
		void RecordCommandBuffers();
		bool CheckAndSelectGPU( std::vector<VkPhysicalDevice> &gpuList );

		void DestroyVKInstance();
		void DestroySwapChain();
		void DestroySemaphores();
		void DestroyDevice();
		void DestroyCommandPool();
		void DestroySurface();

		VkSurfaceFormatKHR GetSwapChainFormat();
		VkImageUsageFlags GetSwapChainUsageFlags( VkSurfaceCapabilitiesKHR vkSurfaceCaps );
		VkExtent2D GetSwapChainExtent( VkSurfaceCapabilitiesKHR vkSurfaceCaps );
		VkSurfaceTransformFlagBitsKHR GetSwapChainTransform( VkSurfaceCapabilitiesKHR vkSurfaceCaps );
		VkPresentModeKHR GetSwapChainPresentMode();

		// Instance and devices
		VkInstance						vkInstance					= VK_NULL_HANDLE;
		VkDevice						vkDevice					= VK_NULL_HANDLE;
		VkPhysicalDevice				selectedGPU					= VK_NULL_HANDLE;
		VkPhysicalDeviceProperties		GPUProperties				= {};
		VkPhysicalDeviceFeatures		GPUFeatures					= {};

		// Swap chain
		VkSemaphore						imageAvailableSemaphore		= VK_NULL_HANDLE;
		VkSemaphore						renderingFinishedSemaphore	= VK_NULL_HANDLE;
		VkSwapchainKHR					vkSwapChain					= VK_NULL_HANDLE;
		VkImageUsageFlags				imageUsageFlags				= VK_NULL_HANDLE;
		VkSurfaceTransformFlagBitsKHR	swapChainTransform;
		VkPresentModeKHR				presentMode;
		VkSurfaceFormatKHR				vkDesiredFormat;
		VkExtent2D						swapChainExtent;

		std::vector<VkImage>			vkImages;
		std::vector<VkImageView>		vkImageViews; 
		std::vector<VkFramebuffer>		vkFramebuffers;

		// Command pool
		VkCommandPool					vkCommandPool				= VK_NULL_HANDLE;
		VkQueue							vkGraphicsQueue				= VK_NULL_HANDLE;
		VkQueue							vkPresentQueue				= VK_NULL_HANDLE;
		uint32_t						graphicsQueueFamilyIndex	= UINT32_MAX;
		uint32_t						presentQueueFamilyIndex		= UINT32_MAX;
		uint32_t						imageCount					= 0;
		std::vector<VkCommandBuffer>	vkCommandBuffers;

		// Surface
		VkSurfaceKHR					vkSurface					= VK_NULL_HANDLE;
		uint32_t						surfaceHeight				= 0;
		uint32_t						surfaceWidth				= 0;

		// Render pass
		VkRenderPass					vkRenderPass				= VK_NULL_HANDLE;
	};
}
