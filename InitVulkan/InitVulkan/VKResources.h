#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW\glfw3.h>
#include <vulkan\vulkan.h>
#include <cstdlib>
#include <assert.h>
#include <vector>

#include "DataStructures.h"
#include "Utils\Helpers.h"

using namespace DataStructures;
using namespace Utils;


namespace VulkanResources
{
	class VKResources
	{
	public:
		VKResources();
		~VKResources();

		// Initiations
		void InitVKInstance();
		void InitDevice();
		void CreateSurface( GLFWwindow * window );
		void CreateSwapChain( int windowHeight, int windowWidth );
		void CreateCommandPool();
		void CreateRenderPass();
		void CreatePipeline();
		void CreateFrameBuffers();
		void OnWindowSizeChanged( GLFWwindow* vkWindow, int windowWidth, int windowHeight );

		// Input buffer creation
		void CreateVertexBuffer( Vertex* vertexData );

		// Draw calls
		uint32_t AcquireImageIndex( GLFWwindow* vkWindow );
		void SubmitBuffers( uint32_t imageIndex, GLFWwindow* vkWindow );

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
		void DestroyPipeline();
		void DestroyCommandPool();
		void DestroySurface();

		// Graphics pipeline initiations
		AutoDeleter<VkShaderModule, PFN_vkDestroyShaderModule> CreateShader( const char* filename );
		AutoDeleter<VkPipelineLayout, PFN_vkDestroyPipelineLayout> CreatePipelineLayout();

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

		uint32_t						imageCount					= 0;
		std::vector<VkImage>			vkImages;
		std::vector<VkImageView>		vkImageViews; 
		std::vector<VkFramebuffer>		vkFramebuffers;

		// Command pool
		VkCommandPool					vkCommandPool				= VK_NULL_HANDLE;
		VkQueue							vkGraphicsQueue				= VK_NULL_HANDLE;
		VkQueue							vkPresentQueue				= VK_NULL_HANDLE;
		uint32_t						graphicsQueueFamilyIndex	= UINT32_MAX;
		uint32_t						presentQueueFamilyIndex		= UINT32_MAX;
		std::vector<VkCommandBuffer>	vkCommandBuffers;

		// Vertex buffer
		VkBuffer						vertexBuffer				= VK_NULL_HANDLE;

		// Surface
		VkSurfaceKHR					vkSurface					= VK_NULL_HANDLE;
		uint32_t						surfaceHeight				= 0;
		uint32_t						surfaceWidth				= 0;

		// Graphics pipelines
		VkRenderPass					vkRenderPass				= VK_NULL_HANDLE;
		VkPipeline						vkPipeline					= VK_NULL_HANDLE;
	};
}
