
#include "Renderer.h"


Renderer::Renderer()
{
	InitVKInstance();
	InitDevice();
}

Renderer::~Renderer()
{
	DestroyDevice();
	DestroyVKInstance();
}

void Renderer::InitVKInstance()
{
	VkApplicationInfo vkApplicationInfo {};
	vkApplicationInfo.sType						= VK_STRUCTURE_TYPE_APPLICATION_INFO;
	vkApplicationInfo.apiVersion				= VK_API_VERSION;
	vkApplicationInfo.applicationVersion		= VK_MAKE_VERSION( 0, 1, 0 );
	vkApplicationInfo.pApplicationName			= "First taste of vulkan";

	VkInstanceCreateInfo vkInstanceCreateInfo {};
	vkInstanceCreateInfo.sType					= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	vkInstanceCreateInfo.pApplicationInfo		= &vkApplicationInfo;

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
	vkInstance = nullptr;
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

	VkDeviceCreateInfo vkDeviceCreateInfo {};
	vkDeviceCreateInfo.sType					= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	vkDeviceCreateInfo.queueCreateInfoCount		= 1;
	vkDeviceCreateInfo.pQueueCreateInfos		= &vkDeviceQueueCreateInfo;
	
	auto err = vkCreateDevice( gpu, &vkDeviceCreateInfo, nullptr, &vkDevice );

	if (err != VK_SUCCESS)
	{
		assert( 0 && "Vulken error: create device failed!" );
		std::exit( -1 );
	}
}

void Renderer::CheckAndSelectGPU( std::vector<VkPhysicalDevice> &gpuList )
{
	bool found = false;

	for (int gpuIndex = 0; gpuIndex < gpuList.size(); gpuIndex++)
	{
		gpu = gpuList[gpuIndex];

		// Get family properties (Features e.g. graphics pipeline, compute pipeline and etc)
		uint32_t familyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties( gpu, &familyCount, nullptr );
		std::vector<VkQueueFamilyProperties> familyPropertyList( familyCount );
		vkGetPhysicalDeviceQueueFamilyProperties( gpu, &familyCount, familyPropertyList.data() );

		// Check which one of these families support graphics bit
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

void Renderer::DestroyDevice()
{
	vkDestroyDevice( vkDevice, nullptr );
	vkDevice = nullptr;
}
