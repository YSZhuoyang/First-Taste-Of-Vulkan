#pragma once

#include <vulkan\vulkan.h>
#include <cstdlib>
#include <assert.h>
#include <vector>


class Renderer
{
public:
	Renderer();
	~Renderer();

private:
	void InitVKInstance();
	void DestroyVKInstance();

	void InitDevice();
	void InitSwapChain();
	void CheckAndSelectGPU( std::vector<VkPhysicalDevice> &gpuList );
	void CreateWindow();
	void DestroyWindow();
	void DestroySwapChain();
	void DestroyDevice();
	void InitCommandPool();
	void DestroyCommandPool();

	VkInstance						vkInstance				= VK_NULL_HANDLE;
	VkDevice						vkDevice				= VK_NULL_HANDLE;
	VkPhysicalDevice				gpu						= VK_NULL_HANDLE;
	VkPhysicalDeviceProperties		gpuProperties			= {};
	VkSwapchainKHR					vkSwapChain				= VK_NULL_HANDLE;

	VkCommandPool					vkCommandPool			= VK_NULL_HANDLE;
	VkQueue							vkQueue					= VK_NULL_HANDLE;
	VkFence							vkFence					= VK_NULL_HANDLE;
	VkSemaphore						vkSemaphore				= VK_NULL_HANDLE;
	VkCommandBuffer					vkCommandBuffers[1];	// 2

	VkSurfaceKHR					vkSurface				= VK_NULL_HANDLE;
	unsigned int					height					= 480;
	unsigned int					width					= 640;

	uint32_t						graphicsFamilyIndex		= 0;
};

