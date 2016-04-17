#pragma once

#include <vulkan\vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>
#include <cstdlib>
#include <assert.h>
#include <vector>

#include "DataStructures.h"
#include "VKResources.h"
#include "VKWindow.h"


using namespace DataStructures;
using namespace VulkanResources;
using namespace GLFWWindowResources;

class Renderer
{
public:
	Renderer();
	~Renderer();

	void Stop();
	bool IsRunning();

private:
	//void CreateWindow();
	//void DestroyWindow();
	/*void InitVKInstance();
	void DestroyVKInstance();

	void CreateCommandPool();
	void CreateCommandBuffers();
	void RecordCommandBuffers();

	void InitDevice();
	void CreateSwapChain();
	void GetSwapChainNext( VkSemaphore presentCompleteSemaphore, uint32_t imageIndex );
	void PresentSwapChain( VkCommandBuffer cmdBuffer, VkQueue queue, uint32_t imageIndex );
	void CheckAndSelectGPU( std::vector<VkPhysicalDevice> &gpuList );
	
	void DestroySwapChain();
	void DestroyDevice();
	void DestroyCommandPool();

	void setImageLayout( VkCommandBuffer cmdBuffer, VkImage image,
		VkImageAspectFlags aspects,
		VkImageLayout oldLayout,
		VkImageLayout newLayout );*/

	void Update();
	void Render();

	/*VkInstance						vkInstance				= VK_NULL_HANDLE;
	VkDevice						vkDevice				= VK_NULL_HANDLE;
	VkPhysicalDevice				gpu						= VK_NULL_HANDLE;
	VkPhysicalDeviceProperties		gpuProperties			= {};
	VkSwapchainKHR					vkSwapChain				= VK_NULL_HANDLE;
	std::vector<SwapChainBuffer>	swapChainBuffers;

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
	uint32_t						surfaceWidth			= 940;*/

	//GLFWwindow*						window					= VK_NULL_HANDLE;

	VKResources*					vkResources				= nullptr;
	VKWindow*						vkWindow				= nullptr;
	bool							isRunning				= true;
};

