#pragma once

#include <vulkan\vulkan.h>

namespace DataStructures
{
	struct Vertex
	{
		float x;
		float y;
		float z;
		float w;
	};

	struct FrameResources
	{
		VkFramebuffer		frameBuffer;
		VkCommandBuffer		commandBuffer;
		VkSemaphore			imageAvailableSemaphore;
		VkSemaphore			renderingFinishedSemaphore;
		VkFence				fence;

		FrameResources() :
			frameBuffer( VK_NULL_HANDLE ),
			commandBuffer( VK_NULL_HANDLE ),
			imageAvailableSemaphore( VK_NULL_HANDLE ),
			renderingFinishedSemaphore( VK_NULL_HANDLE ),
			fence( VK_NULL_HANDLE ) {
		}
	};
}
