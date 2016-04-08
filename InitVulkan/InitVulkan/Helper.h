#pragma once

#include <vulkan\vulkan.h>
#include <assert.h>


namespace Utilities
{
	void errorCheck(VkResult result)
	{
		assert( result == VK_SUCCESS );
	}
}
