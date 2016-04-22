#include "VKResources.h"


using namespace VulkanResources;
using namespace DataStructures;

VKResources::VKResources()
{
	
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
	// Init GLFW
	if (!glfwInit())
	{
		assert( 0 && "GLFW error: init GLFW failed!" );
		std::exit( -1 );
	}

	// check for vulkan support
	if (GLFW_FALSE == glfwVulkanSupported())
	{
		// not supported
		glfwTerminate();
		std::exit( -1 );
	}

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
	CheckAndSelectGPU( gpuList );

	float queuePriorities[] { 1.0f };
	std::vector<const char *> enabledExtensions			= { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	VkDeviceQueueCreateInfo vkDeviceQueueCreateInfo		= {};
	vkDeviceQueueCreateInfo.sType						= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	vkDeviceQueueCreateInfo.queueCount					= 1;
	vkDeviceQueueCreateInfo.pQueuePriorities			= queuePriorities;
	vkDeviceQueueCreateInfo.queueFamilyIndex			= graphicsFamilyIndex;

	VkDeviceCreateInfo vkDeviceCreateInfo				= {};
	vkDeviceCreateInfo.sType							= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	vkDeviceCreateInfo.queueCreateInfoCount				= 1;
	vkDeviceCreateInfo.pQueueCreateInfos				= &vkDeviceQueueCreateInfo;
	vkDeviceCreateInfo.enabledExtensionCount			= enabledExtensions.size();
	vkDeviceCreateInfo.ppEnabledExtensionNames			= enabledExtensions.data();

	auto res = vkCreateDevice( gpu, &vkDeviceCreateInfo, VK_NULL_HANDLE, &vkDevice );

	if (res != VK_SUCCESS)
	{
		assert( 0 && "Vulken error: create device failed!" );
		glfwTerminate();
		std::exit( -1 );
	}

	// The param queue index need to be smaller than the queue count
	vkGetDeviceQueue( vkDevice, graphicsFamilyIndex, 0, &vkQueue );
}

void VKResources::CreateCommandPool()
{
	VkResult res;

	VkCommandPoolCreateInfo vkCommandPoolCreateInfo = {};
	vkCommandPoolCreateInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	vkCommandPoolCreateInfo.pNext					= VK_NULL_HANDLE;
	vkCommandPoolCreateInfo.queueFamilyIndex		= graphicsFamilyIndex;
	vkCommandPoolCreateInfo.flags					= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	res = vkCreateCommandPool( vkDevice, &vkCommandPoolCreateInfo, VK_NULL_HANDLE, &vkCommandPool );
	assert( res == VK_SUCCESS );

	CreateCommandBuffers();
	RecordCommandBuffers();
}

void VKResources::CreateCommandBuffers()
{
	// Need check
	// Retrieve number of images / buffers
	vkGetSwapchainImagesKHR( vkDevice, vkSwapChain, &imageCount, VK_NULL_HANDLE );

	/*if (imageCount == 0)
	{
		assert( 0 && "Vulken error: get swap chain number failed!" );
		glfwTerminate();
		std::exit( -1 );
	}*/

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

	for (uint32_t i = 0; i < imageCount; i++)
	{
		VkImageMemoryBarrier barrier_from_present_to_clear	= {};
		barrier_from_present_to_clear.sType					= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier_from_present_to_clear.pNext					= VK_NULL_HANDLE;
		barrier_from_present_to_clear.srcAccessMask			= VK_ACCESS_MEMORY_READ_BIT;
		barrier_from_present_to_clear.dstAccessMask			= VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier_from_present_to_clear.oldLayout				= VK_IMAGE_LAYOUT_UNDEFINED;
		barrier_from_present_to_clear.newLayout				= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier_from_present_to_clear.srcQueueFamilyIndex	= graphicsFamilyIndex;
		barrier_from_present_to_clear.dstQueueFamilyIndex	= graphicsFamilyIndex;
		barrier_from_present_to_clear.image					= vkImages[i];
		barrier_from_present_to_clear.subresourceRange		= subresources;

		VkImageMemoryBarrier barrier_from_clear_to_present	= {};
		barrier_from_clear_to_present.sType					= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier_from_clear_to_present.pNext					= VK_NULL_HANDLE;
		barrier_from_clear_to_present.srcAccessMask			= VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier_from_clear_to_present.dstAccessMask			= VK_ACCESS_MEMORY_READ_BIT;
		barrier_from_clear_to_present.oldLayout				= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier_from_clear_to_present.newLayout				= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier_from_clear_to_present.srcQueueFamilyIndex	= graphicsFamilyIndex;
		barrier_from_clear_to_present.dstQueueFamilyIndex	= graphicsFamilyIndex;
		barrier_from_clear_to_present.image					= vkImages[i];
		barrier_from_clear_to_present.subresourceRange		= subresources;
	}

	// Create an image view and a framebuffer for each image
	/*for (uint32_t i = 0; i < imageCount; i++)
	{
	VkImageViewCreateInfo vkImageViewCreateInfo				= {};
	vkImageViewCreateInfo.sType								= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	vkImageViewCreateInfo.pNext								= VK_NULL_HANDLE;
	vkImageViewCreateInfo.format							= colorFormat;
	vkImageViewCreateInfo.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
	vkImageViewCreateInfo.subresourceRange.baseMipLevel		= 0;
	vkImageViewCreateInfo.subresourceRange.levelCount		= 1;
	vkImageViewCreateInfo.subresourceRange.baseArrayLayer	= 0;
	vkImageViewCreateInfo.subresourceRange.layerCount		= 1;
	vkImageViewCreateInfo.viewType							= VK_IMAGE_VIEW_TYPE_2D;
	vkImageViewCreateInfo.flags								= 0;
	vkImageViewCreateInfo.components						= { VK_COMPONENT_SWIZZLE_R,
	VK_COMPONENT_SWIZZLE_G,
	VK_COMPONENT_SWIZZLE_B,
	VK_COMPONENT_SWIZZLE_A };

	swapChainBuffers[i].vkImage = vkImages[i];
	setImageLayout( vkCommandBuffer, vkImages[i],
	VK_IMAGE_ASPECT_COLOR_BIT,
	VK_IMAGE_LAYOUT_UNDEFINED,
	VK_IMAGE_LAYOUT_PRESENT_SRC_KHR );
	vkImageViewCreateInfo.image = swapChainBuffers[i].vkImage;

	res = vkCreateImageView( vkDevice, &vkImageViewCreateInfo, VK_NULL_HANDLE, &swapChainBuffers[i].vkImageView );
	assert( res == VK_SUCCESS );

	// Create framebuffer
	VkFramebufferCreateInfo vkFramebufferCreateInfo				= {};
	vkFramebufferCreateInfo.sType								= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	vkFramebufferCreateInfo.attachmentCount						= 1;
	vkFramebufferCreateInfo.width								= swapChainExtent.width;
	vkFramebufferCreateInfo.height								= swapChainExtent.height;
	vkFramebufferCreateInfo.layers								= 1;

	res = vkCreateFramebuffer( vkDevice, &vkFramebufferCreateInfo, VK_NULL_HANDLE, &swapChainBuffers[i].frameBuffer );
	assert( res == VK_SUCCESS );
	}*/
}

void VKResources::CreateWindow()
{
	// Tell GLFW not to create OpenGL context with a window
	glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
	window = glfwCreateWindow( windowWidth, windowHeight, "First taste of Vulkan", nullptr, nullptr );

	VkResult res = glfwCreateWindowSurface( vkInstance, window, nullptr, &vkSurface );

	//glfwGetFramebufferSize( window, &windowWidth, &windowHeight );

	if (res != VK_SUCCESS)
	{
		assert( 0 && "Vulkan error: create surface failed!" );
		glfwTerminate();
		std::exit( -1 );
	}
}

GLFWwindow* VKResources::GetWindowInstance()
{
	return window;
}

void VKResources::DestroyWindow()
{
	glfwDestroyWindow( window );
	glfwTerminate();
	window = nullptr;
}

void VKResources::CreateSwapChain()
{
	VkResult res;

	VkSurfaceCapabilitiesKHR vkSurfaceCaps = {};
	res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR( gpu, vkSurface, &vkSurfaceCaps );

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

	res = vkGetPhysicalDeviceSurfacePresentModesKHR( gpu, vkSurface, &presentModeCount, VK_NULL_HANDLE );
	assert( res == VK_SUCCESS && presentModeCount >= 1 );

	std::vector<VkPresentModeKHR> vkPresentModes( presentModeCount );
	res = vkGetPhysicalDeviceSurfacePresentModesKHR( gpu, vkSurface, &presentModeCount, vkPresentModes.data() );
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

	if (desiredNumberOfImages > vkSurfaceCaps.maxImageCount)
	{
		desiredNumberOfImages = vkSurfaceCaps.maxImageCount;
	}

	// Get image format
	VkFormat colorFormat;
	VkColorSpaceKHR colorSpace;
	uint32_t formatCount = 0;

	vkGetPhysicalDeviceSurfaceFormatsKHR( gpu, vkSurface, &formatCount, VK_NULL_HANDLE );
	std::vector<VkSurfaceFormatKHR> surfaceFormats( formatCount );
	vkGetPhysicalDeviceSurfaceFormatsKHR( gpu, vkSurface, &formatCount, surfaceFormats.data() );

	if (formatCount == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else
	{
		assert( formatCount >= 1 );
		colorFormat = surfaceFormats[0].format;
	}

	colorSpace = surfaceFormats[0].colorSpace;
	VkSwapchainKHR oldSwapChain = vkSwapChain;

	// Create swap chain
	VkSwapchainCreateInfoKHR vkSwapChainCreateInfo		= {};
	vkSwapChainCreateInfo.sType							= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	vkSwapChainCreateInfo.surface						= vkSurface;
	vkSwapChainCreateInfo.minImageCount					= desiredNumberOfImages;
	vkSwapChainCreateInfo.imageFormat					= colorFormat;
	vkSwapChainCreateInfo.imageColorSpace				= colorSpace;
	vkSwapChainCreateInfo.imageExtent					= { swapChainExtent.width, swapChainExtent.height };
	vkSwapChainCreateInfo.presentMode					= vkPresentMode;
	vkSwapChainCreateInfo.imageArrayLayers				= 1;
	vkSwapChainCreateInfo.imageUsage					= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	vkSwapChainCreateInfo.imageSharingMode				= VK_SHARING_MODE_EXCLUSIVE;
	vkSwapChainCreateInfo.queueFamilyIndexCount			= 1;
	vkSwapChainCreateInfo.pQueueFamilyIndices			= { 0 };
	vkSwapChainCreateInfo.preTransform					= VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	vkSwapChainCreateInfo.compositeAlpha				= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	vkSwapChainCreateInfo.oldSwapchain					= oldSwapChain;

	vkCreateSwapchainKHR( vkDevice, &vkSwapChainCreateInfo, VK_NULL_HANDLE, &vkSwapChain );
}

VkInstance VKResources::GetInstance()
{
	return vkInstance;
}

VkSurfaceKHR VKResources::GetSurface()
{
	return vkSurface;
}

void VKResources::GetSwapChainNext( VkSemaphore presentCompleteSemaphore, uint32_t imageIndex )
{
	VkResult res;

	res = vkAcquireNextImageKHR( vkDevice, vkSwapChain, 0, vkSemaphore, (VkFence) 0, &imageIndex );
	assert( res == VK_SUCCESS );
}

void VKResources::CheckAndSelectGPU( std::vector<VkPhysicalDevice> &gpuList )
{
	bool found = false;

	for (unsigned int gpuIndex = 0; gpuIndex < gpuList.size(); gpuIndex++)
	{
		gpu = gpuList[gpuIndex];

		// Get family properties (Features e.g. graphics pipeline, compute pipeline and etc)
		uint32_t familyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties( gpu, &familyCount, VK_NULL_HANDLE );
		std::vector<VkQueueFamilyProperties> familyPropertyList( familyCount );
		vkGetPhysicalDeviceQueueFamilyProperties( gpu, &familyCount, familyPropertyList.data() );

		// Check which one of these families support graphics bit, note that
		// here only check the graphics support, in most cases compute pipeline,
		// transfer, sparse memory support should be checked as well
		for (uint32_t familyIndex = 0; familyIndex < familyCount; familyIndex++)
		{
			if (familyPropertyList[familyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				found = true;
				graphicsFamilyIndex = familyIndex;
				break;
			}
		}

		if (found)
		{
			vkGetPhysicalDeviceProperties( gpu, &gpuProperties );

			break;
		}
	}

	if (!found)
	{
		assert( 0 && "Vulkan error: queue family supporting graphics not found!" );
		glfwTerminate();
		std::exit( -1 );
	}
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
	vkDestroyDevice( vkDevice, VK_NULL_HANDLE );
	vkDevice = VK_NULL_HANDLE;
}

void VKResources::DestroyCommandPool()
{
	vkDestroySemaphore( vkDevice, vkSemaphore, VK_NULL_HANDLE );
	vkSemaphore = VK_NULL_HANDLE;

	vkDestroyFence( vkDevice, vkFence, VK_NULL_HANDLE );
	vkFence = VK_NULL_HANDLE;

	vkDestroyCommandPool( vkDevice, vkCommandPool, VK_NULL_HANDLE );
	vkCommandPool = VK_NULL_HANDLE;
}

void VulkanResources::VKResources::DestroySurface()
{
	vkDestroySurfaceKHR( vkInstance, vkSurface, VK_NULL_HANDLE );
	vkSurface = VK_NULL_HANDLE;
}

/*void VKResources::setImageLayout(
	VkCommandBuffer vkCommandBuffer,
	VkImage image,
	VkImageAspectFlags aspects,
	VkImageLayout oldLayout,
	VkImageLayout newLayout )
{
	// Create image memory barrier which separate actions into two groups:
	// actions performed before the barrier and actions performed after the barrier
	VkImageMemoryBarrier vkImageMemoryBarrier				= {};
	vkImageMemoryBarrier.sType								= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	vkImageMemoryBarrier.oldLayout							= oldLayout;
	vkImageMemoryBarrier.newLayout							= newLayout;
	vkImageMemoryBarrier.image								= image;
	vkImageMemoryBarrier.subresourceRange.aspectMask		= aspects;
	vkImageMemoryBarrier.subresourceRange.baseMipLevel		= 0;
	vkImageMemoryBarrier.subresourceRange.levelCount		= 1;
	vkImageMemoryBarrier.subresourceRange.layerCount		= 1;

	switch (oldLayout)
	{
		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			vkImageMemoryBarrier.srcAccessMask =
				VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			vkImageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			vkImageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			vkImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			vkImageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;

		default:
			break;
	}

	switch (newLayout)
	{
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			vkImageMemoryBarrier.srcAccessMask |= VK_ACCESS_TRANSFER_READ_BIT;
			vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			vkImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			vkImageMemoryBarrier.dstAccessMask |=
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			vkImageMemoryBarrier.srcAccessMask =
				VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;

		default:
			break;
	}

	VkPipelineStageFlagBits srcFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlagBits dstFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	vkCmdPipelineBarrier( vkCommandBuffer, srcFlags, dstFlags, 0, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &vkImageMemoryBarrier );
}*/
