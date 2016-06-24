#include "VKResources.h"


using namespace VulkanResources;
using namespace DataStructures;

VKResources::VKResources()
{
	// Init GLFW
	if (!glfwInit())
	{
		assert( 0 && "GLFW error: init GLFW failed!" );
		std::exit( -1 );
	}

	// check for vulkan support
	if (GLFW_FALSE == glfwVulkanSupported())
	{
		// If not supported
		glfwTerminate();
		std::exit( -1 );
	}
}

VKResources::~VKResources()
{
	DestroyCommandPool();
	DestroySwapChain();
	DestroySurface();
	DestroyDevice();
	DestroyVKInstance();
}

void VKResources::InitVKInstance()
{
	unsigned int instanceExtensionsCount;
	const char** instanceExtensionsBuffer = glfwGetRequiredInstanceExtensions( &instanceExtensionsCount );

	VkApplicationInfo vkApplicationInfo				= {};
	vkApplicationInfo.sType							= VK_STRUCTURE_TYPE_APPLICATION_INFO;
	vkApplicationInfo.apiVersion					= VK_API_VERSION_1_0;
	vkApplicationInfo.applicationVersion			= VK_MAKE_VERSION( 0, 1, 0 );
	vkApplicationInfo.pApplicationName				= "First taste of vulkan";
	vkApplicationInfo.pEngineName					= VK_NULL_HANDLE;

	VkInstanceCreateInfo vkInstanceCreateInfo		= {};
	vkInstanceCreateInfo.sType						= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	vkInstanceCreateInfo.pApplicationInfo			= &vkApplicationInfo;
	vkInstanceCreateInfo.enabledExtensionCount		= instanceExtensionsCount;
	vkInstanceCreateInfo.ppEnabledExtensionNames	= instanceExtensionsBuffer;
	vkInstanceCreateInfo.ppEnabledLayerNames		= VK_NULL_HANDLE;
	vkInstanceCreateInfo.enabledLayerCount			= 0;

	auto res = vkCreateInstance( &vkInstanceCreateInfo, VK_NULL_HANDLE, &vkInstance );

	if (res != VK_SUCCESS)
	{
		assert( 0 && "Vulken error: create instance failed!" );
		std::exit( -1 );
	}
}

void VKResources::DestroyVKInstance()
{
	vkDestroyInstance( vkInstance, VK_NULL_HANDLE );
	vkInstance = VK_NULL_HANDLE;
}

void VKResources::InitDevice()
{
	// Get physical device handle
	uint32_t gpuCount = 0;
	vkEnumeratePhysicalDevices( vkInstance, &gpuCount, VK_NULL_HANDLE );	// Return how many GPUs I have
	std::vector<VkPhysicalDevice> gpuList( gpuCount );
	vkEnumeratePhysicalDevices( vkInstance, &gpuCount, gpuList.data() );	// Return all gpu handles

	// Select a device (GPU)
	if (!CheckAndSelectGPU( gpuList ))
	{
		assert( 0 && "Vulkan error: queue family supporting graphics not found!" );
		glfwTerminate();
		std::exit( -1 );
	}

	std::vector<float> queuePriorities					= { 1.0f };
	std::vector<const char *> enabledExtensions			= { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	VkDeviceQueueCreateInfo vkDeviceQueueCreateInfo		= {};
	vkDeviceQueueCreateInfo.sType						= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	vkDeviceQueueCreateInfo.queueCount					= static_cast<uint32_t>(queuePriorities.size());
	vkDeviceQueueCreateInfo.pQueuePriorities			= queuePriorities.data();
	vkDeviceQueueCreateInfo.queueFamilyIndex			= graphicsQueueFamilyIndex;

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	queueCreateInfos.push_back( vkDeviceQueueCreateInfo );

	VkDeviceCreateInfo vkDeviceCreateInfo				= {};
	vkDeviceCreateInfo.sType							= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	vkDeviceCreateInfo.queueCreateInfoCount				= static_cast<uint32_t>(queueCreateInfos.size());
	vkDeviceCreateInfo.pQueueCreateInfos				= queueCreateInfos.data();
	vkDeviceCreateInfo.enabledExtensionCount			= static_cast<uint32_t>(enabledExtensions.size());
	vkDeviceCreateInfo.ppEnabledExtensionNames			= enabledExtensions.data();

	auto res = vkCreateDevice( selectedGPU, &vkDeviceCreateInfo, VK_NULL_HANDLE, &vkDevice );

	if (res != VK_SUCCESS)
	{
		assert( 0 && "Vulken error: create device failed!" );
		glfwTerminate();
		std::exit( -1 );
	}

	// The param queue index need to be smaller than the queue count
	vkGetDeviceQueue( vkDevice, graphicsQueueFamilyIndex, 0, &vkGraphicsQueue );
	vkGetDeviceQueue( vkDevice, presentQueueFamilyIndex, 0, &vkPresentQueue );

	// Create semaphores
	CreateSemaphores();
}

void VKResources::CreateCommandPool()
{
	VkResult res;

	VkCommandPoolCreateInfo vkCommandPoolCreateInfo = {};
	vkCommandPoolCreateInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	vkCommandPoolCreateInfo.pNext					= VK_NULL_HANDLE;
	vkCommandPoolCreateInfo.queueFamilyIndex		= presentQueueFamilyIndex;
	vkCommandPoolCreateInfo.flags					= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	res = vkCreateCommandPool( vkDevice, &vkCommandPoolCreateInfo, VK_NULL_HANDLE, &vkCommandPool );
	assert( res == VK_SUCCESS );

	CreateCommandBuffers();
	RecordCommandBuffers();
}

void VKResources::PresentQueue( uint32_t imageIndex )
{
	// Submit image
	VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;

	VkSubmitInfo submitInfo					= {};
	submitInfo.sType						= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext						= VK_NULL_HANDLE;
	submitInfo.waitSemaphoreCount			= 1;
	submitInfo.pWaitSemaphores				= &imageAvailableSemaphore;
	submitInfo.pWaitDstStageMask			= &waitDstStageMask;
	submitInfo.commandBufferCount			= 1;
	submitInfo.pCommandBuffers				= &vkCommandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount			= 1;
	submitInfo.pSignalSemaphores			= &renderingFinishedSemaphore;

	vkQueueSubmit( vkPresentQueue, 1, &submitInfo, VK_NULL_HANDLE );
	
	// Present image
	VkPresentInfoKHR presentInfo			= {};
	presentInfo.sType						= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext						= VK_NULL_HANDLE;
	presentInfo.waitSemaphoreCount			= 1;
	presentInfo.pWaitSemaphores				= &renderingFinishedSemaphore;
	presentInfo.swapchainCount				= 1;
	presentInfo.pSwapchains					= &vkSwapChain;
	presentInfo.pImageIndices				= &imageIndex;
	presentInfo.pResults					= VK_NULL_HANDLE;

	VkResult result = vkQueuePresentKHR( vkPresentQueue, &presentInfo );

	switch (result)
	{
		case VK_SUCCESS:
			break;

		case VK_SUBOPTIMAL_KHR:
			break;

		case VK_ERROR_OUT_OF_DATE_KHR:
			// To be added
			//OnWindowSizeChanged();
			break;

		default:
			assert( 0 && "Problem occurred during swap chain image acquisition!" );
			break;
	}
}

uint32_t VKResources::AcquireImage()
{
	uint32_t image_index;
	VkResult result = vkAcquireNextImageKHR(
		vkDevice,
		vkSwapChain,
		UINT64_MAX,
		imageAvailableSemaphore,
		VK_NULL_HANDLE,
		&image_index );

	switch (result)
	{
		case VK_SUCCESS:
			break;
		
		case VK_SUBOPTIMAL_KHR:
			break;

		case VK_ERROR_OUT_OF_DATE_KHR:
			// To be added
			//OnWindowSizeChanged();
			break;

		default:
			assert( 0 && "Problem occurred during swap chain image acquisition!" );
			break;
	}

	return image_index;
}

void VKResources::CreateSemaphores()
{
	// Need error check
	VkSemaphoreCreateInfo vkSemaphoreCreateInfo	= {};
	vkSemaphoreCreateInfo.sType					= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkSemaphoreCreateInfo.pNext					= VK_NULL_HANDLE;
	vkSemaphoreCreateInfo.flags					= 0;

	vkCreateSemaphore( vkDevice, &vkSemaphoreCreateInfo, VK_NULL_HANDLE, &imageAvailableSemaphore );
	vkCreateSemaphore( vkDevice, &vkSemaphoreCreateInfo, VK_NULL_HANDLE, &renderingFinishedSemaphore );
}

void VKResources::CreateCommandBuffers()
{
	// Need check
	// Retrieve number of images / buffers
	vkGetSwapchainImagesKHR( vkDevice, vkSwapChain, &imageCount, VK_NULL_HANDLE );

	if (imageCount == 0)
	{
		assert( 0 && "Vulken error: get swap chain number failed!" );
		glfwTerminate();
		std::exit( -1 );
	}

	vkCommandBuffers.resize( imageCount );

	VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo {};
	vkCommandBufferAllocateInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	vkCommandBufferAllocateInfo.pNext				= VK_NULL_HANDLE;
	vkCommandBufferAllocateInfo.commandBufferCount	= imageCount;
	vkCommandBufferAllocateInfo.commandPool			= vkCommandPool;
	vkCommandBufferAllocateInfo.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	// Need check
	vkAllocateCommandBuffers( vkDevice, &vkCommandBufferAllocateInfo, vkCommandBuffers.data() );
}

void VKResources::RecordCommandBuffers()
{
	VkResult res;

	std::vector<VkImage> vkImages( imageCount );
	swapChainBuffers.resize( imageCount );

	res = vkGetSwapchainImagesKHR( vkDevice, vkSwapChain, &imageCount, vkImages.data() );
	assert( res == VK_SUCCESS );

	VkImageSubresourceRange subresources	= {};
	subresources.aspectMask					= VK_IMAGE_ASPECT_COLOR_BIT;
	subresources.baseMipLevel				= 0;
	subresources.levelCount					= 1;
	subresources.layerCount					= 1;

	VkCommandBufferBeginInfo vkCommandBufferBeginInfo		= {};
	vkCommandBufferBeginInfo.sType							= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkCommandBufferBeginInfo.pNext							= VK_NULL_HANDLE;
	vkCommandBufferBeginInfo.flags							= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	vkCommandBufferBeginInfo.pInheritanceInfo				= VK_NULL_HANDLE;

	VkClearColorValue clearColor =
	{
		{ 1.0f, 0.8f, 0.4f, 0.0f }
	};

	for (uint32_t i = 0; i < imageCount; i++)
	{
		VkImageMemoryBarrier barrier_from_present_to_clear	= {};
		barrier_from_present_to_clear.sType					= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier_from_present_to_clear.pNext					= VK_NULL_HANDLE;
		barrier_from_present_to_clear.srcAccessMask			= VK_ACCESS_MEMORY_READ_BIT;
		barrier_from_present_to_clear.dstAccessMask			= VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier_from_present_to_clear.oldLayout				= VK_IMAGE_LAYOUT_UNDEFINED;
		barrier_from_present_to_clear.newLayout				= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier_from_present_to_clear.srcQueueFamilyIndex	= presentQueueFamilyIndex;
		barrier_from_present_to_clear.dstQueueFamilyIndex	= presentQueueFamilyIndex;
		barrier_from_present_to_clear.image					= vkImages[i];
		barrier_from_present_to_clear.subresourceRange		= subresources;

		VkImageMemoryBarrier barrier_from_clear_to_present	= {};
		barrier_from_clear_to_present.sType					= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier_from_clear_to_present.pNext					= VK_NULL_HANDLE;
		barrier_from_clear_to_present.srcAccessMask			= VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier_from_clear_to_present.dstAccessMask			= VK_ACCESS_MEMORY_READ_BIT;
		barrier_from_clear_to_present.oldLayout				= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier_from_clear_to_present.newLayout				= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier_from_clear_to_present.srcQueueFamilyIndex	= presentQueueFamilyIndex;
		barrier_from_clear_to_present.dstQueueFamilyIndex	= presentQueueFamilyIndex;
		barrier_from_clear_to_present.image					= vkImages[i];
		barrier_from_clear_to_present.subresourceRange		= subresources;

		vkBeginCommandBuffer( vkCommandBuffers[i], &vkCommandBufferBeginInfo );
		
		vkCmdPipelineBarrier(
			vkCommandBuffers[i],
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1,
			&barrier_from_present_to_clear );
		vkCmdClearColorImage(
			vkCommandBuffers[i],
			vkImages[i],
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			&clearColor, 1, &subresources );
		vkCmdPipelineBarrier(
			vkCommandBuffers[i],
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			0, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1,
			&barrier_from_clear_to_present );

		res = vkEndCommandBuffer( vkCommandBuffers[i] );

		if (res != VK_SUCCESS)
		{
			assert( 0 && "Vulken error: could not record command buffers!" );
			std::exit( -1 );
		}
	}
}

void VKResources::CreateSurface( GLFWwindow* window )
{
	VkResult res = glfwCreateWindowSurface( vkInstance, window, nullptr, &vkSurface );

	//glfwGetFramebufferSize( window, &windowWidth, &windowHeight );

	if (res != VK_SUCCESS)
	{
		assert( 0 && "Vulkan error: create surface failed!" );
		glfwTerminate();
		std::exit( -1 );
	}
}

VkSurfaceFormatKHR VKResources::GetSwapChainFormat( std::vector<VkSurfaceFormatKHR> &surfaceFormats )
{
	if (surfaceFormats.size() == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		return { VK_FORMAT_R8G8B8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
	}
	else
	{
		assert( surfaceFormats.size() >= 1 );

		for (VkSurfaceFormatKHR &surfaceFormat : surfaceFormats)
		{
			if (surfaceFormat.format == VK_FORMAT_R8G8B8A8_UNORM)
			{
				return surfaceFormat;
			}
		}

		return surfaceFormats[0];
	}
}

void VKResources::CreateSwapChain( int windowHeight, int windowWidth )
{
	VkResult res;

	if (vkDevice != VK_NULL_HANDLE)
	{
		vkDeviceWaitIdle( vkDevice );
	}

	VkSurfaceCapabilitiesKHR vkSurfaceCaps = {};
	res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR( selectedGPU, vkSurface, &vkSurfaceCaps );

	if (res != VK_SUCCESS)
	{
		assert( 0 && "Vulken error: get physical device surface cap failed!" );
		glfwTerminate();
		std::exit( -1 );
	}

	VkExtent2D swapChainExtent = {};

	if (swapChainExtent.height == -1 && swapChainExtent.width == -1)
	{
		swapChainExtent.width = windowWidth;
		swapChainExtent.height = windowHeight;
	}
	else
	{
		swapChainExtent = vkSurfaceCaps.currentExtent;
	}

	// Get present modes
	uint32_t presentModeCount = 0;

	res = vkGetPhysicalDeviceSurfacePresentModesKHR( selectedGPU, vkSurface, &presentModeCount, VK_NULL_HANDLE );
	assert( res == VK_SUCCESS && presentModeCount >= 1 );

	std::vector<VkPresentModeKHR> vkPresentModes( presentModeCount );
	res = vkGetPhysicalDeviceSurfacePresentModesKHR( selectedGPU, vkSurface, &presentModeCount, vkPresentModes.data() );
	assert( res == VK_SUCCESS );

	VkPresentModeKHR vkPresentMode = VK_PRESENT_MODE_FIFO_KHR;

	for (uint32_t i = 0; i < presentModeCount; i++)
	{
		if (vkPresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			vkPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}

		if (vkPresentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			vkPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}

	// Get image count for swap chain
	assert( vkSurfaceCaps.maxImageCount >= 1 );

	uint32_t desiredNumberOfImages = vkSurfaceCaps.minImageCount + 1;

	if (vkSurfaceCaps.maxImageCount > 0 && desiredNumberOfImages > vkSurfaceCaps.maxImageCount)
	{
		desiredNumberOfImages = vkSurfaceCaps.maxImageCount;
	}

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR( selectedGPU, vkSurface, &formatCount, VK_NULL_HANDLE );
	std::vector<VkSurfaceFormatKHR> surfaceFormats( formatCount );
	vkGetPhysicalDeviceSurfaceFormatsKHR( selectedGPU, vkSurface, &formatCount, surfaceFormats.data() );

	// Get image format and color space
	VkSurfaceFormatKHR vkDesiredFormat = GetSwapChainFormat( surfaceFormats );
	VkSwapchainKHR oldSwapChain = vkSwapChain;

	// Create swap chain
	VkSwapchainCreateInfoKHR vkSwapChainCreateInfo		= {};
	vkSwapChainCreateInfo.sType							= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	vkSwapChainCreateInfo.surface						= vkSurface;
	vkSwapChainCreateInfo.minImageCount					= desiredNumberOfImages;
	vkSwapChainCreateInfo.imageFormat					= vkDesiredFormat.format;
	vkSwapChainCreateInfo.imageColorSpace				= vkDesiredFormat.colorSpace;
	vkSwapChainCreateInfo.imageExtent					= { swapChainExtent.width, swapChainExtent.height };
	vkSwapChainCreateInfo.presentMode					= vkPresentMode;
	vkSwapChainCreateInfo.imageArrayLayers				= 1;
	vkSwapChainCreateInfo.imageUsage					= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	vkSwapChainCreateInfo.imageSharingMode				= VK_SHARING_MODE_EXCLUSIVE;
	vkSwapChainCreateInfo.queueFamilyIndexCount			= 0;
	vkSwapChainCreateInfo.pQueueFamilyIndices			= VK_NULL_HANDLE;
	vkSwapChainCreateInfo.preTransform					= VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	vkSwapChainCreateInfo.compositeAlpha				= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	vkSwapChainCreateInfo.clipped						= VK_TRUE;
	vkSwapChainCreateInfo.oldSwapchain					= oldSwapChain;

	vkCreateSwapchainKHR( vkDevice, &vkSwapChainCreateInfo, VK_NULL_HANDLE, &vkSwapChain );

	if (oldSwapChain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR( vkDevice, oldSwapChain, VK_NULL_HANDLE );
	}
}

VkInstance VKResources::GetInstance()
{
	return vkInstance;
}

VkSurfaceKHR VKResources::GetSurface()
{
	return vkSurface;
}

bool VKResources::CheckAndSelectGPU( std::vector<VkPhysicalDevice> &gpuList )
{
	for (VkPhysicalDevice gpu : gpuList)
	{
		vkGetPhysicalDeviceProperties( gpu, &GPUProperties );
		vkGetPhysicalDeviceFeatures( gpu, &GPUFeatures );

		uint32_t majorVersion = VK_VERSION_MAJOR( GPUProperties.apiVersion );
		uint32_t minorVersion = VK_VERSION_MINOR( GPUProperties.apiVersion );
		uint32_t patchVersion = VK_VERSION_PATCH( GPUProperties.apiVersion );

		if (majorVersion < 1 && GPUProperties.limits.maxImageDimension2D < 4096)
		{
			printf( "Physical device %s doesn't support required parameters!", GPUProperties.deviceName );
			continue;
		}

		selectedGPU = gpu;

		// Get family properties (Features e.g. graphics pipeline, compute pipeline and etc)
		uint32_t familyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties( gpu, &familyCount, VK_NULL_HANDLE );
		std::vector<VkQueueFamilyProperties> familyPropertyList( familyCount );
		vkGetPhysicalDeviceQueueFamilyProperties( gpu, &familyCount, familyPropertyList.data() );

		std::vector<VkBool32> queuePresentSupport( familyCount );

		// Check which one of these families support graphics bit, note that
		// here only check the graphics support, in most cases compute pipeline,
		// transfer, sparse memory support should be checked as well
		for (uint32_t familyIndex = 0; familyIndex < familyCount; familyIndex++)
		{
			vkGetPhysicalDeviceSurfaceSupportKHR( gpu, familyIndex, vkSurface, &queuePresentSupport[familyIndex] );

			if (queuePresentSupport[familyIndex] &&
				familyPropertyList[familyIndex].queueCount > 0 &&
				(familyPropertyList[familyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				graphicsQueueFamilyIndex = familyIndex;
				presentQueueFamilyIndex = familyIndex;

				return true;
			}
		}
	}

	return false;
}

void VKResources::DestroySwapChain()
{
	// Clear all images and image views
	for (SwapChainBuffer buffer : swapChainBuffers)
	{
		vkDestroyImageView( vkDevice, buffer.vkImageView, VK_NULL_HANDLE );
		vkDestroyImage( vkDevice, buffer.vkImage, VK_NULL_HANDLE );
	}

	vkDestroySwapchainKHR( vkDevice, vkSwapChain, VK_NULL_HANDLE );
	vkSwapChain = VK_NULL_HANDLE;
}

void VKResources::DestroyDevice()
{
	vkDeviceWaitIdle( vkDevice );
	vkDestroyDevice( vkDevice, VK_NULL_HANDLE );
	vkDevice = VK_NULL_HANDLE;
}

void VKResources::DestroyCommandPool()
{
	vkDestroySemaphore( vkDevice, imageAvailableSemaphore, VK_NULL_HANDLE );
	imageAvailableSemaphore = VK_NULL_HANDLE;

	vkDestroySemaphore( vkDevice, renderingFinishedSemaphore, VK_NULL_HANDLE );
	renderingFinishedSemaphore = VK_NULL_HANDLE;

	vkDestroyFence( vkDevice, vkFence, VK_NULL_HANDLE );
	vkFence = VK_NULL_HANDLE;

	vkDestroyCommandPool( vkDevice, vkCommandPool, VK_NULL_HANDLE );
	vkCommandPool = VK_NULL_HANDLE;
}

void VKResources::DestroySurface()
{
	vkDestroySurfaceKHR( vkInstance, vkSurface, VK_NULL_HANDLE );
	vkSurface = VK_NULL_HANDLE;
}
