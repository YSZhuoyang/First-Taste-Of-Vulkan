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
	void CheckAndSelectGPU( std::vector<VkPhysicalDevice> &gpuList );
	void CreateWindow();
	void DestroyWindow();
	void DestroyDevice();

	VkInstance						vkInstance				= nullptr;
	VkDevice						vkDevice				= nullptr;
	VkPhysicalDevice				gpu						= nullptr;
	VkPhysicalDeviceProperties		gpuProperties			= {};
	VkSurfaceKHR					vkSurface				= NULL;

	uint32_t						graphicsFamilyIndex		= 0;
};

