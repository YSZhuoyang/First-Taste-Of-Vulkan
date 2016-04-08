#define GLFW_INCLUDE_VULKAN

#include "Renderer.h"
#include "DataStructures.h"

#include <GLFW\glfw3.h>


using namespace DataStructures;

Renderer::Renderer()
{
	InitVKInstance();
	InitDevice();
	CreateWindow();
	InitSwapChain();
	InitCommandPool();
}

Renderer::~Renderer()
{
	DestroyWindow();
	DestroyCommandPool();
	DestroySwapChain();
	DestroyDevice();
	DestroyVKInstance();
}

void Renderer::InitVKInstance()
{
	// Init GLFW
	if (!glfwInit())
	{
		assert( 0 && "GLFW error: init GLFW failed!" );
		std::exit( -1 );
	}

	unsigned int extCount;
	const char** extensions = glfwGetRequiredInstanceExtensions( &extCount );

	VkApplicationInfo vkApplicationInfo {};
	vkApplicationInfo.sType							= VK_STRUCTURE_TYPE_APPLICATION_INFO;
	vkApplicationInfo.apiVersion					= VK_API_VERSION_1_0;
	vkApplicationInfo.applicationVersion			= VK_MAKE_VERSION( 0, 1, 0 );
	vkApplicationInfo.pApplicationName				= "First taste of vulkan";
	vkApplicationInfo.pEngineName					= nullptr;

	/*std::vector<const char*> enabledExtensions;
	enabledExtensions.push_back( VK_KHR_SURFACE_EXTENSION_NAME );
#if defined( _WIN32 )
	enabledExtensions.push_back( VK_KHR_WIN32_SURFACE_EXTENSION_NAME );
#else
	enabledExtensions.push_back( VK_KHR_XCB_SURFACE_EXTENSION_NAME );
#endif
	*/

	VkInstanceCreateInfo vkInstanceCreateInfo {};
	vkInstanceCreateInfo.sType						= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	vkInstanceCreateInfo.pApplicationInfo			= &vkApplicationInfo;
	vkInstanceCreateInfo.enabledExtensionCount		= extCount;
	vkInstanceCreateInfo.ppEnabledExtensionNames	= extensions;
	vkInstanceCreateInfo.ppEnabledLayerNames		= nullptr;
	vkInstanceCreateInfo.enabledLayerCount			= 0;

	auto err = vkCreateInstance( &vkInstanceCreateInfo, nullptr, &vkInstance );

	if (err != VK_SUCCESS)
	{
		assert( 0 && "Vulken error: create instance failed!" );
		std::exit( -1 );
	}
}

void Renderer::DestroyVKInstance()
{
	vkDestroyInstance( vkInstance, nullptr );
	vkInstance = VK_NULL_HANDLE;
}

void Renderer::InitDevice()
{
	// Get physical device handle
	uint32_t gpuCount = 0;
	vkEnumeratePhysicalDevices( vkInstance, &gpuCount, nullptr );	// Return how many GPUs I have
	std::vector<VkPhysicalDevice> gpuList( gpuCount );
	vkEnumeratePhysicalDevices( vkInstance, &gpuCount, gpuList.data() );	// Return all gpu handles

	// Select a device (GPU)
	CheckAndSelectGPU( gpuList );

	float queuePriorities[] { 1.0f };

	VkDeviceQueueCreateInfo vkDeviceQueueCreateInfo {};
	vkDeviceQueueCreateInfo.sType				= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	vkDeviceQueueCreateInfo.queueCount			= 1;
	vkDeviceQueueCreateInfo.pQueuePriorities	= queuePriorities;
	vkDeviceQueueCreateInfo.queueFamilyIndex	= graphicsFamilyIndex;

	std::vector<const char *> enabledExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	VkDeviceCreateInfo vkDeviceCreateInfo {};
	vkDeviceCreateInfo.sType					= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	vkDeviceCreateInfo.queueCreateInfoCount		= 1;
	vkDeviceCreateInfo.pQueueCreateInfos		= &vkDeviceQueueCreateInfo;
	vkDeviceCreateInfo.enabledExtensionCount	= enabledExtensions.size();
	vkDeviceCreateInfo.ppEnabledExtensionNames	= enabledExtensions.data();

	auto err = vkCreateDevice( gpu, &vkDeviceCreateInfo, nullptr, &vkDevice );

	if (err != VK_SUCCESS)
	{
		assert( 0 && "Vulken error: create device failed!" );
		std::exit( -1 );
	}

	// The param queue index need to be smaller than the queue count
	vkGetDeviceQueue( vkDevice, graphicsFamilyIndex, 0, &vkQueue );
}

void Renderer::InitSwapChain()
{
	VkResult res;

	VkSurfaceCapabilitiesKHR vkSurfaceCaps {};
	res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR( gpu, vkSurface, &vkSurfaceCaps );

	if (res != VK_SUCCESS)
	{
		assert( 0 && "Vulken error: get physical device surface cap failed!" );
		std::exit( -1 );
	}

	VkExtent2D swapChainExtent = {};

	if (swapChainExtent.height == -1 && swapChainExtent.width == -1)
	{
		swapChainExtent.width = width;
		swapChainExtent.height = height;
	}
	else
	{
		swapChainExtent = vkSurfaceCaps.currentExtent;
	}

	// Get present modes
	uint32_t presentModeCount = 0;

	res = vkGetPhysicalDeviceSurfacePresentModesKHR( gpu, vkSurface, &presentModeCount, nullptr );
	assert( res == VK_SUCCESS && presentModeCount >= 1 );

	std::vector<VkPresentModeKHR> vkPresentModes( presentModeCount );
	res = vkGetPhysicalDeviceSurfacePresentModesKHR( gpu, vkSurface, &presentModeCount, vkPresentModes.data() );
	assert( res == VK_SUCCESS );

	VkPresentModeKHR vkPresentMode = VK_PRESENT_MODE_FIFO_KHR;

	for (int i = 0; i < presentModeCount; i++)
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

	assert( vkSurfaceCaps.maxImageCount >= 1 );

	uint32_t imageCount = vkSurfaceCaps.minImageCount + 1;

	if (imageCount > vkSurfaceCaps.maxImageCount)
	{
		imageCount = vkSurfaceCaps.maxImageCount;
	}

	// Get image format
	VkFormat colorFormat;
	VkColorSpaceKHR colorSpace;
	uint32_t formatCount = 0;

	vkGetPhysicalDeviceSurfaceFormatsKHR( gpu, vkSurface, &formatCount, nullptr );
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

	// Create swap chain
	VkSwapchainCreateInfoKHR vkSwapChainCreateInfo = {};
	vkSwapChainCreateInfo.sType					= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	vkSwapChainCreateInfo.surface				= vkSurface;
	vkSwapChainCreateInfo.minImageCount			= imageCount;
	vkSwapChainCreateInfo.imageFormat			= colorFormat;
	vkSwapChainCreateInfo.imageColorSpace		= colorSpace;
	vkSwapChainCreateInfo.imageExtent			=	{ swapChainExtent.width, swapChainExtent.height };
	vkSwapChainCreateInfo.presentMode			= vkPresentMode;
	vkSwapChainCreateInfo.imageArrayLayers		= 1;
	vkSwapChainCreateInfo.imageUsage			= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	vkSwapChainCreateInfo.imageSharingMode		= VK_SHARING_MODE_EXCLUSIVE;
	vkSwapChainCreateInfo.queueFamilyIndexCount = 1;
	vkSwapChainCreateInfo.pQueueFamilyIndices	= { 0 };
	vkSwapChainCreateInfo.preTransform			= VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	vkSwapChainCreateInfo.compositeAlpha		= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	res = vkCreateSwapchainKHR( vkDevice, &vkSwapChainCreateInfo, nullptr, &vkSwapChain );
	assert( res == VK_SUCCESS );

	std::vector<VkImage> vkImages(imageCount);
	std::vector<SwapChainBuffer> swapChainBuffers(imageCount);

	res = vkGetSwapchainImagesKHR( vkDevice, vkSwapChain, &imageCount, vkImages.data() );
	assert( res == VK_SUCCESS );

	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType					= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.pNext					= VK_NULL_HANDLE;
	imageViewCreateInfo.format					= colorFormat;
	imageViewCreateInfo.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewCreateInfo.subresourceRange.baseMipLevel	= 0;
	imageViewCreateInfo.subresourceRange.levelCount		= 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount		= 1;
	imageViewCreateInfo.viewType				= VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.flags					= 0;
	imageViewCreateInfo.components				= { VK_COMPONENT_SWIZZLE_R,
													VK_COMPONENT_SWIZZLE_G,
													VK_COMPONENT_SWIZZLE_B,
													VK_COMPONENT_SWIZZLE_A };


}

void Renderer::CheckAndSelectGPU( std::vector<VkPhysicalDevice> &gpuList )
{
	bool found = false;

	for (unsigned int gpuIndex = 0; gpuIndex < gpuList.size(); gpuIndex++)
	{
		gpu = gpuList[gpuIndex];

		// Get family properties (Features e.g. graphics pipeline, compute pipeline and etc)
		uint32_t familyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties( gpu, &familyCount, nullptr );
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
		std::exit( -1 );
	}
}

void Renderer::CreateWindow()
{
	glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
	GLFWwindow* window = glfwCreateWindow( width, height, "First taste of Vulkan", nullptr, nullptr );

	VkResult err = glfwCreateWindowSurface( vkInstance, window, nullptr, &vkSurface );

	if (err)
	{
		assert( 0 && "Vulkan error: create surface failed!" );
		std::exit( -1 );
	}
}

void Renderer::DestroyWindow()
{
	vkDestroySurfaceKHR( vkInstance, vkSurface, nullptr );
	vkSurface = VK_NULL_HANDLE;
	glfwDestroyWindow( glfwGetCurrentContext() );
}

void Renderer::DestroySwapChain()
{
	vkDestroySwapchainKHR( vkDevice, vkSwapChain, nullptr );
}

void Renderer::DestroyDevice()
{
	vkDestroyDevice( vkDevice, nullptr );
	vkDevice = VK_NULL_HANDLE;
}

void Renderer::InitCommandPool()
{
	VkCommandPoolCreateInfo vkCommandPoolCreateInfo {};
	vkCommandPoolCreateInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	vkCommandPoolCreateInfo.queueFamilyIndex		= graphicsFamilyIndex;
	vkCommandPoolCreateInfo.flags					= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	vkCreateCommandPool( vkDevice, &vkCommandPoolCreateInfo, nullptr, &vkCommandPool );

	// Allocate command buffers
	VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo {};
	vkCommandBufferAllocateInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	vkCommandBufferAllocateInfo.commandBufferCount	= 1;	// 2
	vkCommandBufferAllocateInfo.commandPool			= vkCommandPool;
	vkCommandBufferAllocateInfo.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	/* -------------- Set up the first command buffer ----------------- */
	VkCommandBufferBeginInfo vkCommandBufferBeginInfo {};
	vkCommandBufferBeginInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	// Command buffer state is set to initial state
	vkAllocateCommandBuffers( vkDevice, &vkCommandBufferAllocateInfo, vkCommandBuffers );

	// Command buffer state changes to recording state
	vkBeginCommandBuffer( vkCommandBuffers[0], &vkCommandBufferBeginInfo );

	// Viewport need to be set up for every command buffer!!
	VkViewport vkViewport;
	vkViewport.height								= height;
	vkViewport.width								= width;
	vkViewport.maxDepth								= 1.0f;
	vkViewport.minDepth								= 0.0f;
	vkViewport.x									= 0.0f;
	vkViewport.y									= 0.0f;

	vkCmdSetViewport( vkCommandBuffers[0], 0, 1, &vkViewport );

	// Command buffer state changes to executable state
	vkEndCommandBuffer( vkCommandBuffers[0] );

	/* -------------- Set up the second command buffer ----------------- */
	/*VkCommandBufferBeginInfo vkCommandBufferBeginInfo {};
	vkCommandBufferBeginInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	// Command buffer state is set to initial state
	vkAllocateCommandBuffers( vkDevice, &vkCommandBufferAllocateInfo, &vkCommandBuffers[1] );

	// Command buffer state changes to recording state
	vkBeginCommandBuffer( vkCommandBuffers[1], &vkCommandBufferBeginInfo );

	// Viewport need to be set up for every command buffer!!
	VkViewport vkViewport;
	vkViewport.height								= height;
	vkViewport.width								= width;
	vkViewport.maxDepth								= 1.0f;
	vkViewport.minDepth								= 0.0f;
	vkViewport.x									= 0.0f;
	vkViewport.y									= 0.0f;

	vkCmdSetViewport( vkCommandBuffers[1], 0, 1, &vkViewport );

	// Command buffer state changes to executable state
	vkEndCommandBuffer( vkCommandBuffers[1] );
	*/

	// Create semaphore which tells the gpu when other processes on gpu 
	// have completed, which means it lets gpu to handle queue submissions
	VkSemaphoreCreateInfo vkSemaphoreCreateInfo {};
	vkSemaphoreCreateInfo.sType						= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	vkCreateSemaphore( vkDevice, &vkSemaphoreCreateInfo, nullptr, &vkSemaphore );

	// Create fence which is used for sychronize gpu command submission
	// Fence wait for the gpu to be ready for the cpu side
	VkFenceCreateInfo vkFenceCreateInfo {};
	vkFenceCreateInfo.sType							= VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	vkCreateFence( vkDevice, &vkFenceCreateInfo, nullptr, &vkFence );

	/* ------------------ First submit ------------------- */
	VkSubmitInfo vkSubmitInfo {};
	vkSubmitInfo.sType								= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	vkSubmitInfo.commandBufferCount					= 1;
	vkSubmitInfo.pCommandBuffers					= &vkCommandBuffers[0];
	//vkSubmitInfo.signalSemaphoreCount				= 1;
	//vkSubmitInfo.pWaitSemaphores					= &vkSemaphore;

	//vkQueueSubmit( vkQueue, 1, &vkSubmitInfo, VK_NULL_HANDLE );
	vkQueueSubmit( vkQueue, 1, &vkSubmitInfo, vkFence );

	/* ------------------ Second submit ------------------- */
	// Means the second submission will wait for the first submission to complete
	// all commands
	/*VkPipelineStageFlags vkPipelineStageFlags[] { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };

	VkSubmitInfo vkSubmitInfo {};
	vkSubmitInfo.sType								= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	vkSubmitInfo.commandBufferCount					= 1;
	vkSubmitInfo.pCommandBuffers					= &vkCommandBuffers[1];
	vkSubmitInfo.signalSemaphoreCount				= 1;
	vkSubmitInfo.pWaitSemaphores					= &vkSemaphore;
	vkSubmitInfo.pWaitDstStageMask					= vkPipelineStageFlags;

	vkQueueSubmit( vkQueue, 1, &vkSubmitInfo, VK_NULL_HANDLE );
	*/

	//vkQueueWaitIdle( vkQueue );
	auto res = vkWaitForFences( vkDevice, 1, &vkFence, VK_TRUE, UINT64_MAX );
}

void Renderer::DestroyCommandPool()
{
	vkDestroySemaphore( vkDevice, vkSemaphore, nullptr );
	vkSemaphore = VK_NULL_HANDLE;

	vkDestroyFence( vkDevice, vkFence, nullptr );
	vkFence = VK_NULL_HANDLE;

	vkDestroyCommandPool( vkDevice, vkCommandPool, nullptr );
	vkCommandPool = VK_NULL_HANDLE;
}
