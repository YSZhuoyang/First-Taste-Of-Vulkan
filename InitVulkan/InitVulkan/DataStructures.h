#pragma once

#include <vulkan\vulkan.h>

namespace DataStructures
{
	struct SwapChainBuffer
	{
		VkImage vkImage;
		VkImageView vkImageView;
	};
}
