#define GLFW_INCLUDE_VULKAN

#include "Renderer.h"

#include <GLFW\glfw3.h>


Renderer::Renderer()
{
	InitVKInstance();
	InitDevice();
	InitCommandPool();
	CreateWindow();
}

Renderer::~Renderer()
{
	DestroyWindow();
	DestroyCommandPool();
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
	vkApplicationInfo.apiVersion					= VK_API_VERSION;
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
	VkSurfaceCapabilitiesKHR vkSurfaceCaps {};
	auto res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR( gpu, vkSurface, &vkSurfaceCaps );

	if (res != VK_SUCCESS)
	{
		assert( 0 && "Vulken error: get physical device surface cap failed!" );
		std::exit( -1 );
	}
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
	vkCommandBufferAllocateInfo.commandBufferCount	= 2;
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
	VkCommandBufferBeginInfo vkCommandBufferBeginInfo {};
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
	vkSubmitInfo.signalSemaphoreCount				= 1;
	vkSubmitInfo.pWaitSemaphores					= &vkSemaphore;

	vkQueueSubmit( vkQueue, 1, &vkSubmitInfo, VK_NULL_HANDLE );

	/* ------------------ Second submit ------------------- */
	// Means the second submission will wait for the first submission to complete
	// all commands
	VkPipelineStageFlags vkPipelineStageFlags[] { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };

	VkSubmitInfo vkSubmitInfo {};
	vkSubmitInfo.sType								= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	vkSubmitInfo.commandBufferCount					= 1;
	vkSubmitInfo.pCommandBuffers					= &vkCommandBuffers[1];
	vkSubmitInfo.signalSemaphoreCount				= 1;
	vkSubmitInfo.pWaitSemaphores					= &vkSemaphore;
	vkSubmitInfo.pWaitDstStageMask					= vkPipelineStageFlags;

	vkQueueSubmit( vkQueue, 1, &vkSubmitInfo, VK_NULL_HANDLE );

	vkQueueWaitIdle( vkQueue );

	//auto res = vkWaitForFences( vkDevice, 1, &vkFence, VK_TRUE, UINT64_MAX );
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
