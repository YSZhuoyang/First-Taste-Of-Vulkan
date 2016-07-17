#include "VKResources.h"

using namespace VulkanResources;

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
	vkDeviceWaitIdle( vkDevice );

	DestroyFrameResources();
	DestroyPipeline();
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

}

void VKResources::CreateCommandPool()
{
	VkResult res;

	VkCommandPoolCreateInfo vkCommandPoolCreateInfo = {};
	vkCommandPoolCreateInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	vkCommandPoolCreateInfo.pNext					= VK_NULL_HANDLE;
	vkCommandPoolCreateInfo.queueFamilyIndex		= presentQueueFamilyIndex;
	vkCommandPoolCreateInfo.flags					= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | 
														VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

	res = vkCreateCommandPool( vkDevice, &vkCommandPoolCreateInfo, VK_NULL_HANDLE, &vkCommandPool );
	assert( res == VK_SUCCESS );

	CreateCommandBuffers();
	RecordCommandBuffers();
}

void VKResources::SubmitBuffers( uint32_t imageIndex, GLFWwindow* window )
{
	// Submit image
	VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;

	VkSubmitInfo submitInfo					= {};
	submitInfo.sType						= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext						= VK_NULL_HANDLE;
	submitInfo.waitSemaphoreCount			= 1;
	submitInfo.pWaitSemaphores				= &frameResources[imageIndex].imageAvailableSemaphore;
	submitInfo.pWaitDstStageMask			= &waitDstStageMask;
	submitInfo.commandBufferCount			= 1;
	submitInfo.pCommandBuffers				= &frameResources[imageIndex].commandBuffer;
	//submitInfo.pCommandBuffers				= &vkCommandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount			= 1;
	submitInfo.pSignalSemaphores			= &frameResources[imageIndex].renderingFinishedSemaphore;

	vkQueueSubmit( vkPresentQueue, 1, &submitInfo, VK_NULL_HANDLE );
	
	// Present image
	VkPresentInfoKHR presentInfo			= {};
	presentInfo.sType						= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext						= VK_NULL_HANDLE;
	presentInfo.waitSemaphoreCount			= 1;
	presentInfo.pWaitSemaphores				= &frameResources[imageIndex].renderingFinishedSemaphore;
	presentInfo.swapchainCount				= 1;
	presentInfo.pSwapchains					= &vkSwapChain;
	presentInfo.pImageIndices				= &imageIndex;
	presentInfo.pResults					= VK_NULL_HANDLE;

	VkResult result = vkQueuePresentKHR( vkPresentQueue, &presentInfo );

	switch (result)
	{
		case VK_SUCCESS:
			break;

		case VK_ERROR_OUT_OF_DATE_KHR:
			break;

		case VK_SUBOPTIMAL_KHR:
			int windowWidth;
			int windowHeight;

			glfwGetWindowSize( window, &windowWidth, &windowHeight );
			OnWindowSizeChanged( window, windowWidth, windowHeight );

			break;

		default:
			printf( "Problem occurred during swap chain image acquisition!" );

			break;
	}
}

void VKResources::CreateRenderPass()
{
	VkResult res;

	VkAttachmentDescription vkAttachmentDesc	= {};
	vkAttachmentDesc.flags						= 0;
	vkAttachmentDesc.format						= vkDesiredFormat.format;
	vkAttachmentDesc.loadOp						= VK_ATTACHMENT_LOAD_OP_CLEAR;		// Clear image before specifying the color
	vkAttachmentDesc.storeOp					= VK_ATTACHMENT_STORE_OP_STORE;		// Store image before display it on screen
	vkAttachmentDesc.samples					= VK_SAMPLE_COUNT_1_BIT;			// No multi sampling
	vkAttachmentDesc.stencilLoadOp				= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	vkAttachmentDesc.stencilStoreOp				= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	vkAttachmentDesc.initialLayout				= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	vkAttachmentDesc.finalLayout				= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	std::vector<VkAttachmentDescription> vkAttachmentDesciptions;
	vkAttachmentDesciptions.push_back( vkAttachmentDesc );

	VkAttachmentReference colorAttachmentRef	= {};
	colorAttachmentRef.attachment				= 0;
	colorAttachmentRef.layout					= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	std::vector<VkAttachmentReference> colorAttachmentRefs;
	colorAttachmentRefs.push_back( colorAttachmentRef );

	/************** Subpass descriptions **************/
	VkSubpassDescription vkSubpassDesc		= {};
	vkSubpassDesc.flags						= 0;
	vkSubpassDesc.pipelineBindPoint			= VK_PIPELINE_BIND_POINT_GRAPHICS;
	vkSubpassDesc.inputAttachmentCount		= 0;
	vkSubpassDesc.pInputAttachments			= VK_NULL_HANDLE;
	vkSubpassDesc.colorAttachmentCount		= 1;
	vkSubpassDesc.pColorAttachments			= colorAttachmentRefs.data();
	vkSubpassDesc.pResolveAttachments		= VK_NULL_HANDLE;
	vkSubpassDesc.preserveAttachmentCount	= 0;
	vkSubpassDesc.pPreserveAttachments		= VK_NULL_HANDLE;

	std::vector<VkSubpassDescription> subpassDescriptions;
	subpassDescriptions.push_back( vkSubpassDesc );

	/*************** Subpass dependency ***************/
	VkSubpassDependency subpassDepSrc	= {};
	subpassDepSrc.srcSubpass			= VK_SUBPASS_EXTERNAL;
	subpassDepSrc.dstSubpass			= 0;
	subpassDepSrc.srcStageMask			= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDepSrc.dstStageMask			= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDepSrc.srcAccessMask			= VK_ACCESS_MEMORY_READ_BIT;
	subpassDepSrc.dstAccessMask			= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDepSrc.dependencyFlags		= VK_DEPENDENCY_BY_REGION_BIT;

	VkSubpassDependency subpassDepDst	= {};
	subpassDepDst.srcSubpass			= 0;
	subpassDepDst.dstSubpass			= VK_SUBPASS_EXTERNAL;
	subpassDepDst.srcStageMask			= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDepDst.dstStageMask			= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDepDst.srcAccessMask			= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDepDst.dstAccessMask			= VK_ACCESS_MEMORY_READ_BIT;
	subpassDepDst.dependencyFlags		= VK_DEPENDENCY_BY_REGION_BIT;

	std::vector<VkSubpassDependency> subpassDependencies;
	subpassDependencies.push_back( subpassDepSrc );
	subpassDependencies.push_back( subpassDepDst );

	/************* Create render pass **************/
	VkRenderPassCreateInfo vkRenderPassCreateInfo	= {};
	vkRenderPassCreateInfo.sType					= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	vkRenderPassCreateInfo.pNext					= VK_NULL_HANDLE;
	vkRenderPassCreateInfo.flags					= 0;
	vkRenderPassCreateInfo.attachmentCount			= 1;
	vkRenderPassCreateInfo.pAttachments				= vkAttachmentDesciptions.data();
	vkRenderPassCreateInfo.subpassCount				= 1;
	vkRenderPassCreateInfo.pSubpasses				= subpassDescriptions.data();
	vkRenderPassCreateInfo.dependencyCount			= static_cast<uint32_t>(subpassDependencies.size());
	vkRenderPassCreateInfo.pDependencies			= subpassDependencies.data();

	res = vkCreateRenderPass( vkDevice, &vkRenderPassCreateInfo, VK_NULL_HANDLE, &vkRenderPass );

	if (res != VK_SUCCESS)
	{
		printf( "Create renderpass failed" );
		glfwTerminate();
	}
}

void VKResources::CreateFrameBuffers()
{
	VkResult res;
	printf( "Creating frame buffers" );
	VkComponentMapping components =
	{
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY
	};

	VkImageSubresourceRange subsourceRange = 
	{
		VK_IMAGE_ASPECT_COLOR_BIT, 
		0, 
		1, 
		0, 
		1
	};

	for (uint32_t i = 0; i < commandBufferCount; i++)
	{
		VkImageViewCreateInfo imageViewCreateInfo	= {};
		imageViewCreateInfo.sType					= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.pNext					= VK_NULL_HANDLE;
		imageViewCreateInfo.flags					= 0;
		imageViewCreateInfo.image					= vkImages[i];
		imageViewCreateInfo.viewType				= VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format					= vkDesiredFormat.format;
		imageViewCreateInfo.components				= components;
		imageViewCreateInfo.subresourceRange		= subsourceRange;

		res = vkCreateImageView( vkDevice, &imageViewCreateInfo, VK_NULL_HANDLE, &vkImageViews[i] );

		if (res != VK_SUCCESS)
		{
			printf( "Create the %dnd image view failed", i );
			glfwTerminate();
		}

		VkFramebufferCreateInfo frameBufferCreateInfo	= {};
		frameBufferCreateInfo.sType						= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.pNext						= VK_NULL_HANDLE;
		frameBufferCreateInfo.renderPass				= vkRenderPass;
		frameBufferCreateInfo.flags						= 0;
		frameBufferCreateInfo.attachmentCount			= 1;
		frameBufferCreateInfo.pAttachments				= &vkImageViews[i];
		frameBufferCreateInfo.width						= surfaceWidth;
		frameBufferCreateInfo.height					= surfaceHeight;
		frameBufferCreateInfo.layers					= 1;

		res = vkCreateFramebuffer( vkDevice, &frameBufferCreateInfo, VK_NULL_HANDLE, &frameResources[i].frameBuffer );//&vkFramebuffers[i]

		if (res != VK_SUCCESS)
		{
			printf( "Create the %dnd framebuffer failed", i );
			glfwTerminate();
		}
	}
}

uint32_t VKResources::AcquireImageIndex( GLFWwindow* window )
{
	static size_t resource_index = 0;
	FrameResources &currentFrameResource = frameResources[resource_index];

	resource_index = (resource_index + 1) % commandBufferCount;

	vkWaitForFences( vkDevice, 1, &currentFrameResource.fence, VK_FALSE, 1000000000 );
	vkResetFences( vkDevice, 1, &currentFrameResource.fence );

	uint32_t image_index;
	VkResult result = vkAcquireNextImageKHR(
		vkDevice,
		vkSwapChain,
		UINT64_MAX,
		currentFrameResource.imageAvailableSemaphore,
		VK_NULL_HANDLE,
		&image_index
	);

	switch (result)
	{
		case VK_SUCCESS:
			break;

		case VK_SUBOPTIMAL_KHR:
			break;

		case VK_ERROR_OUT_OF_DATE_KHR:
			int windowWidth;
			int windowHeight;

			glfwGetWindowSize( window, &windowWidth, &windowHeight );
			OnWindowSizeChanged( window, windowWidth, windowHeight );

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

	for (uint32_t i = 0; i < commandBufferCount; i++)
	{
		vkCreateSemaphore( vkDevice, &vkSemaphoreCreateInfo, VK_NULL_HANDLE, &frameResources[i].imageAvailableSemaphore );
		vkCreateSemaphore( vkDevice, &vkSemaphoreCreateInfo, VK_NULL_HANDLE, &frameResources[i].renderingFinishedSemaphore );
	}
}

void VKResources::CreateFences()
{
	VkFenceCreateInfo fenceCreateInfo	= {};
	fenceCreateInfo.sType				= VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags				= 0;
	fenceCreateInfo.pNext				= VK_NULL_HANDLE;

	for (uint32_t i = 0; i < commandBufferCount; i++)
	{
		vkCreateFence( vkDevice, &fenceCreateInfo, VK_NULL_HANDLE, &frameResources[i].fence );
	}
}

void VKResources::CreateCommandBuffers()
{
	// Need check
	// Retrieve number of images / buffers

	/*if (imageCount == 0)
	{
		assert( 0 && "Vulken error: get swap chain number failed!" );
		glfwTerminate();
		std::exit( -1 );
	}

	vkCommandBuffers.resize( imageCount );*/

	VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo	= {};
	vkCommandBufferAllocateInfo.sType						= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	vkCommandBufferAllocateInfo.pNext						= VK_NULL_HANDLE;
	vkCommandBufferAllocateInfo.commandBufferCount			= 1;
	vkCommandBufferAllocateInfo.commandPool					= vkCommandPool;
	vkCommandBufferAllocateInfo.level						= VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	for (uint32_t i = 0; i < commandBufferCount; i++)
	{
		// Need check
		vkAllocateCommandBuffers( vkDevice, &vkCommandBufferAllocateInfo, &frameResources[i].commandBuffer );
	}
}

void VKResources::RecordCommandBuffers()
{
	//VkResult res;

	VkImageSubresourceRange subresources					= {};
	subresources.aspectMask									= VK_IMAGE_ASPECT_COLOR_BIT;
	subresources.baseMipLevel								= 0;
	subresources.levelCount									= 1;
	subresources.layerCount									= 1;

	VkCommandBufferBeginInfo vkCommandBufferBeginInfo		= {};
	vkCommandBufferBeginInfo.sType							= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkCommandBufferBeginInfo.pNext							= VK_NULL_HANDLE;
	vkCommandBufferBeginInfo.flags							= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	vkCommandBufferBeginInfo.pInheritanceInfo				= VK_NULL_HANDLE;

	VkClearValue clearColor =
	{
		{ 1.0f, 0.8f, 0.4f, 0.0f }
	};

	for (uint32_t i = 0; i < commandBufferCount; i++)
	{
		vkBeginCommandBuffer( frameResources[i].commandBuffer, &vkCommandBufferBeginInfo );

		if (vkGraphicsQueue != vkPresentQueue)
		{
			VkImageMemoryBarrier barrier_from_present_to_draw = {
				VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     // VkStructureType                sType
				VK_NULL_HANDLE,                             // const void                    *pNext
				VK_ACCESS_MEMORY_READ_BIT,                  // VkAccessFlags                  srcAccessMask
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,       // VkAccessFlags                  dstAccessMask
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,            // VkImageLayout                  oldLayout
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,            // VkImageLayout                  newLayout
				presentQueueFamilyIndex,			        // uint32_t                       srcQueueFamilyIndex
				graphicsQueueFamilyIndex,					// uint32_t                       dstQueueFamilyIndex
				vkImages[i],								// VkImage                        image
				subresources								// VkImageSubresourceRange        subresourceRange
			};

			vkCmdPipelineBarrier( frameResources[i].commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &barrier_from_present_to_draw );
		}

		VkRenderPassBeginInfo render_pass_begin_info = {
			VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,		// VkStructureType                sType
			VK_NULL_HANDLE,									// const void                    *pNext
			vkRenderPass,									// VkRenderPass                   renderPass
			frameResources[i].frameBuffer,					// VkFramebuffer                  framebuffer
			{												// VkRect2D                       renderArea
				{                                           // VkOffset2D                     offset
					0,                                      // int32_t                        x
					0                                       // int32_t                        y
				},
				{                                           // VkExtent2D                     extent
					surfaceWidth,                           // int32_t                        width
					surfaceHeight,                          // int32_t                        height
				}
			},
			1,												// uint32_t                       clearValueCount
			&clearColor										// const VkClearValue            *pClearValues
		};

		vkCmdBeginRenderPass( frameResources[i].commandBuffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE );
		vkCmdBindPipeline( frameResources[i].commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline );
		vkCmdDraw( frameResources[i].commandBuffer, 3, 1, 0, 0 );
		vkCmdEndRenderPass( frameResources[i].commandBuffer );

		if (vkGraphicsQueue != vkPresentQueue)
		{
			VkImageMemoryBarrier barrier_from_draw_to_present = {
				VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,       // VkStructureType              sType
				VK_NULL_HANDLE,                               // const void                  *pNext
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,         // VkAccessFlags                srcAccessMask
				VK_ACCESS_MEMORY_READ_BIT,                    // VkAccessFlags                dstAccessMask
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,              // VkImageLayout                oldLayout
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,              // VkImageLayout                newLayout
				graphicsQueueFamilyIndex,					  // uint32_t                     srcQueueFamilyIndex
				presentQueueFamilyIndex,					  // uint32_t                     dstQueueFamilyIndex
				vkImages[i],				                  // VkImage                      image
				subresources					              // VkImageSubresourceRange      subresourceRange
			};

			vkCmdPipelineBarrier( frameResources[i].commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &barrier_from_draw_to_present );
		}
		
		if (vkEndCommandBuffer( frameResources[i].commandBuffer ) != VK_SUCCESS)
		{
			printf( "Could not record command buffer!" );
			exit( 0 );
		}
	}
}

void VKResources::OnWindowSizeChanged( GLFWwindow* window, int windowWidth, int windowHeight )
{
	printf( "Window resize callback invoked: size: %d, %d", windowWidth, windowHeight );

	vkDeviceWaitIdle( vkDevice );

	// Free command buffers
	DestroyCommandPool();

	// Re-create swap chain
	CreateSwapChain( windowWidth, windowHeight );
	CreateRenderPass();
	CreateFrameBuffers();
	CreateCommandPool();
}

void VKResources::SetupVertexBuffer( Vertex* vertexData )
{
	vertexMemorySize = sizeof( vertexData );

	printf( "Vertex buffer size: %d", vertexMemorySize );

	VkBufferCreateInfo bufferCreateInfo		= {};
	bufferCreateInfo.sType					= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.flags					= 0;
	bufferCreateInfo.pNext					= VK_NULL_HANDLE;
	bufferCreateInfo.size					= vertexMemorySize;
	bufferCreateInfo.usage					= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferCreateInfo.sharingMode			= VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.pQueueFamilyIndices	= VK_NULL_HANDLE;
	bufferCreateInfo.queueFamilyIndexCount	= 0;

	vkCreateBuffer( vkDevice, &bufferCreateInfo, VK_NULL_HANDLE, &vertexBuffer );

	/**************** Allocate buffer memory ****************/
	VkMemoryRequirements vertBufferMemoryRequirement;
	vkGetBufferMemoryRequirements( vkDevice, vertexBuffer, &vertBufferMemoryRequirement );

	VkPhysicalDeviceMemoryProperties vertMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties( selectedGPU, &vertMemoryProperties );

	bool memoryGot = false;

	for (uint32_t i = 0; i < vertMemoryProperties.memoryTypeCount; i++)
	{
		if ((vertBufferMemoryRequirement.memoryTypeBits & (1 << i)) &&
			// Make the memory host visible which enable applications read and write access to it, 
			// with some cost of performance
			(vertMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
		{
			VkMemoryAllocateInfo memoryAllocateInfo	= {};
			memoryAllocateInfo.sType				= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memoryAllocateInfo.pNext				= VK_NULL_HANDLE;
			memoryAllocateInfo.memoryTypeIndex		= i;
			memoryAllocateInfo.allocationSize		= vertBufferMemoryRequirement.size;

			vkAllocateMemory( vkDevice, &memoryAllocateInfo, VK_NULL_HANDLE, &vertexBufferMemory );
			memoryGot = true;
			break;
		}
	}

	if (!memoryGot)
	{
		printf( "Allocate vertex buffer memory failed" );
		exit( 0 );
	}

	/*************** Bind the buffer with the buffer memory ***************/
	vkBindBufferMemory( vkDevice, vertexBuffer, vertexBufferMemory, 0 );

	/*************** Acquire a pointer to this memory ***************/
	void * vertBufferMemoPointer;
	vkMapMemory( vkDevice, vertexBufferMemory, 0, vertexMemorySize, 0, &vertBufferMemoPointer );

	/*************** Upload vertex data ***************/
	memcpy( vertBufferMemoPointer, vertexData, vertexMemorySize );
	VkMappedMemoryRange mappedMemoryRange	= {};
	mappedMemoryRange.sType					= VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedMemoryRange.pNext					= VK_NULL_HANDLE;
	mappedMemoryRange.offset				= 0;
	mappedMemoryRange.memory				= vertexBufferMemory;
	mappedMemoryRange.size					= VK_WHOLE_SIZE;

	// Tell the driver which part of the memory was modified
	vkFlushMappedMemoryRanges( vkDevice, 1, &mappedMemoryRange );
	vkUnmapMemory( vkDevice, vertexBufferMemory );
}

AutoDeleter<VkShaderModule, PFN_vkDestroyShaderModule> VKResources::CreateShader( const char* filename )
{
	VkResult res;
	const std::vector<char> code = GetBinaryFileContents( filename );

	if (code.size() == 0)
	{
		return AutoDeleter<VkShaderModule, PFN_vkDestroyShaderModule>();
	}

	VkShaderModuleCreateInfo shaderCreateInfo	= {};
	shaderCreateInfo.sType						= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderCreateInfo.codeSize					= code.size();
	shaderCreateInfo.pCode						= reinterpret_cast<const uint32_t*>(code.data());
	shaderCreateInfo.flags						= 0;
	shaderCreateInfo.pNext						= VK_NULL_HANDLE;

	VkShaderModule shader;
	res = vkCreateShaderModule( vkDevice, &shaderCreateInfo, VK_NULL_HANDLE, &shader );

	if (res != VK_SUCCESS)
	{
		printf( "Shader creation failed from file %s", filename );

		return AutoDeleter<VkShaderModule, PFN_vkDestroyShaderModule>();
	}

	return AutoDeleter<VkShaderModule, PFN_vkDestroyShaderModule>( shader, vkDestroyShaderModule, vkDevice );
}

AutoDeleter<VkPipelineLayout, PFN_vkDestroyPipelineLayout> VKResources::CreatePipelineLayout()
{
	VkPipelineLayoutCreateInfo layoutCreateInfo	= {};
	layoutCreateInfo.sType						= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutCreateInfo.flags						= 0;
	layoutCreateInfo.pNext						= VK_NULL_HANDLE;
	layoutCreateInfo.setLayoutCount				= 0;
	layoutCreateInfo.pSetLayouts				= VK_NULL_HANDLE;
	layoutCreateInfo.pPushConstantRanges		= VK_NULL_HANDLE;

	VkPipelineLayout pipelineLayout;
	vkCreatePipelineLayout( vkDevice, &layoutCreateInfo, VK_NULL_HANDLE, &pipelineLayout );

	return AutoDeleter<VkPipelineLayout, PFN_vkDestroyPipelineLayout>( pipelineLayout, vkDestroyPipelineLayout, vkDevice );
}

void VKResources::CreatePipeline()
{
	VkResult res;

	AutoDeleter<VkShaderModule, PFN_vkDestroyShaderModule> vertexShader =
		CreateShader( "Shaders/vert.spv" );
	AutoDeleter<VkShaderModule, PFN_vkDestroyShaderModule> fragmentShader =
		CreateShader( "Shaders/frag.spv" );

	if (!vertexShader || !fragmentShader)
	{
		printf( "Creating shaders failed" );
		glfwTerminate();
	}

	VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo	= {};
	vertexShaderStageCreateInfo.sType							= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStageCreateInfo.stage							= VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderStageCreateInfo.pName							= "main";
	vertexShaderStageCreateInfo.pNext							= VK_NULL_HANDLE;
	vertexShaderStageCreateInfo.module							= vertexShader.Get();
	vertexShaderStageCreateInfo.flags							= 0;
	vertexShaderStageCreateInfo.pSpecializationInfo				= VK_NULL_HANDLE;

	VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo	= {};
	fragShaderStageCreateInfo.sType								= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageCreateInfo.stage								= VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageCreateInfo.pName								= "main";
	fragShaderStageCreateInfo.pNext								= VK_NULL_HANDLE;
	fragShaderStageCreateInfo.module							= fragmentShader.Get();
	fragShaderStageCreateInfo.flags								= 0;
	fragShaderStageCreateInfo.pSpecializationInfo				= VK_NULL_HANDLE;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos;
	shaderStageCreateInfos.push_back( vertexShaderStageCreateInfo );
	shaderStageCreateInfos.push_back( fragShaderStageCreateInfo );

	/***************** Vertex input binding & attribute descrition *****************/
	VkVertexInputBindingDescription vertBindingDesc	= {};
	vertBindingDesc.stride							= sizeof( Vertex );
	vertBindingDesc.binding							= 0;
	vertBindingDesc.inputRate						= VK_VERTEX_INPUT_RATE_VERTEX;

	std::vector<VkVertexInputBindingDescription> vertBindingDescriptions( 1 );
	vertBindingDescriptions.push_back( vertBindingDesc );

	VkVertexInputAttributeDescription vertAttriDesc	= {};
	vertAttriDesc.binding							= vertBindingDesc.binding;
	vertAttriDesc.format							= VK_FORMAT_R32G32B32A32_SFLOAT;
	vertAttriDesc.location							= 0;
	vertAttriDesc.offset							= offsetof( Vertex, x );

	std::vector<VkVertexInputAttributeDescription> vertAttriDescriptions( 1 );
	vertAttriDescriptions.push_back( vertAttriDesc );

	VkPipelineVertexInputStateCreateInfo vertexStateCreateInfo	= {};
	vertexStateCreateInfo.sType									= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexStateCreateInfo.flags									= 0;
	vertexStateCreateInfo.pNext									= VK_NULL_HANDLE;
	vertexStateCreateInfo.vertexAttributeDescriptionCount		= static_cast<uint32_t>(vertAttriDescriptions.size());
	vertexStateCreateInfo.pVertexAttributeDescriptions			= vertAttriDescriptions.data();
	vertexStateCreateInfo.vertexBindingDescriptionCount			= static_cast<uint32_t>(vertBindingDescriptions.size());
	vertexStateCreateInfo.pVertexBindingDescriptions			= vertBindingDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo assemblyStateCreateInfo	= {};
	assemblyStateCreateInfo.sType									= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	assemblyStateCreateInfo.pNext									= VK_NULL_HANDLE;
	assemblyStateCreateInfo.flags									= 0;
	assemblyStateCreateInfo.primitiveRestartEnable					= VK_FALSE;
	assemblyStateCreateInfo.topology								= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	// Prepare view port
	//VkViewport viewport = { 0.0f, 0.0f, (float) surfaceWidth, (float) surfaceHeight, 0.0f, 1.0f };
	//VkRect2D rect2D = { {0, 0}, {surfaceWidth, surfaceHeight} };

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo	= {};
	viewportStateCreateInfo.sType								= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.flags								= 0;
	viewportStateCreateInfo.pNext								= VK_NULL_HANDLE;
	viewportStateCreateInfo.viewportCount						= 1;
	viewportStateCreateInfo.pViewports							= VK_NULL_HANDLE;
	viewportStateCreateInfo.scissorCount						= 1;
	viewportStateCreateInfo.pScissors							= VK_NULL_HANDLE;

	std::vector<VkDynamicState> dynamicStates;
	dynamicStates.push_back( VK_DYNAMIC_STATE_VIEWPORT );
	dynamicStates.push_back( VK_DYNAMIC_STATE_SCISSOR );

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo	= {};
	dynamicStateCreateInfo.sType							= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.flags							= 0;
	dynamicStateCreateInfo.dynamicStateCount				= static_cast<uint32_t>(dynamicStates.size());
	dynamicStateCreateInfo.pDynamicStates					= dynamicStates.data();
	dynamicStateCreateInfo.pNext							= VK_NULL_HANDLE;

	// Prepare rasterization state
	VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo	= {};
	rasterizationStateCreateInfo.sType									= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationStateCreateInfo.flags									= 0;
	rasterizationStateCreateInfo.pNext									= VK_NULL_HANDLE;
	rasterizationStateCreateInfo.cullMode								= VK_CULL_MODE_BACK_BIT;
	rasterizationStateCreateInfo.depthBiasEnable						= VK_FALSE;
	rasterizationStateCreateInfo.depthBiasConstantFactor				= 0.0f;
	rasterizationStateCreateInfo.depthClampEnable						= VK_FALSE;
	rasterizationStateCreateInfo.depthBiasClamp							= 0.0f;
	rasterizationStateCreateInfo.lineWidth								= 1.0f;
	rasterizationStateCreateInfo.polygonMode							= VK_POLYGON_MODE_FILL;
	rasterizationStateCreateInfo.rasterizerDiscardEnable				= VK_FALSE;

	// Prepare multi-sampling state
	VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo	= {};
	multisampleStateCreateInfo.sType								= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleStateCreateInfo.flags								= 0;
	multisampleStateCreateInfo.pNext								= VK_NULL_HANDLE;
	multisampleStateCreateInfo.minSampleShading						= 1.0f;
	multisampleStateCreateInfo.rasterizationSamples					= VK_SAMPLE_COUNT_1_BIT;
	multisampleStateCreateInfo.sampleShadingEnable					= VK_FALSE;
	multisampleStateCreateInfo.alphaToOneEnable						= VK_FALSE;
	multisampleStateCreateInfo.alphaToCoverageEnable				= VK_FALSE;
	multisampleStateCreateInfo.pSampleMask							= VK_NULL_HANDLE;

	// Prepare blending state
	VkPipelineColorBlendAttachmentState blendAttachmentState	= {};
	blendAttachmentState.blendEnable							= VK_FALSE;
	blendAttachmentState.srcAlphaBlendFactor					= VK_BLEND_FACTOR_ONE;
	blendAttachmentState.dstAlphaBlendFactor					= VK_BLEND_FACTOR_ZERO;
	blendAttachmentState.alphaBlendOp							= VK_BLEND_OP_ADD;
	blendAttachmentState.srcColorBlendFactor					= VK_BLEND_FACTOR_ONE;
	blendAttachmentState.dstColorBlendFactor					= VK_BLEND_FACTOR_ZERO;
	blendAttachmentState.colorBlendOp							= VK_BLEND_OP_ADD;
	blendAttachmentState.colorWriteMask							= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | 
																VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo	= {};
	colorBlendStateCreateInfo.sType									= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendStateCreateInfo.flags									= 0;
	colorBlendStateCreateInfo.pNext									= VK_NULL_HANDLE;
	colorBlendStateCreateInfo.logicOpEnable							= VK_FALSE;
	colorBlendStateCreateInfo.logicOp								= VK_LOGIC_OP_COPY;
	colorBlendStateCreateInfo.pAttachments							= &blendAttachmentState;
	colorBlendStateCreateInfo.attachmentCount						= 1;
	colorBlendStateCreateInfo.blendConstants[0]						= 0.0f;
	colorBlendStateCreateInfo.blendConstants[1]						= 0.0f;
	colorBlendStateCreateInfo.blendConstants[2]						= 0.0f;
	colorBlendStateCreateInfo.blendConstants[3]						= 0.0f;

	// Prepare pipeline layout
	AutoDeleter<VkPipelineLayout, PFN_vkDestroyPipelineLayout> pipelineLayout = CreatePipelineLayout();

	if (!pipelineLayout)
	{
		printf( "Pipeline layout create failed" );
		glfwTerminate();
		exit( 0 );
	}

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo	= {};
	graphicsPipelineCreateInfo.sType						= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.flags						= 0;
	graphicsPipelineCreateInfo.pNext						= VK_NULL_HANDLE;
	graphicsPipelineCreateInfo.layout						= pipelineLayout.Get();
	graphicsPipelineCreateInfo.stageCount					= static_cast<uint32_t>(shaderStageCreateInfos.size());
	graphicsPipelineCreateInfo.pStages						= shaderStageCreateInfos.data();
	graphicsPipelineCreateInfo.pVertexInputState			= &vertexStateCreateInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState			= &assemblyStateCreateInfo;
	graphicsPipelineCreateInfo.pTessellationState			= VK_NULL_HANDLE;
	graphicsPipelineCreateInfo.pViewportState				= &viewportStateCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState			= &rasterizationStateCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState			= &multisampleStateCreateInfo;
	graphicsPipelineCreateInfo.pDepthStencilState			= VK_NULL_HANDLE;
	graphicsPipelineCreateInfo.pColorBlendState				= &colorBlendStateCreateInfo;
	graphicsPipelineCreateInfo.pDynamicState				= &dynamicStateCreateInfo;
	graphicsPipelineCreateInfo.renderPass					= vkRenderPass;
	graphicsPipelineCreateInfo.subpass						= 0;
	graphicsPipelineCreateInfo.basePipelineHandle			= VK_NULL_HANDLE;
	graphicsPipelineCreateInfo.basePipelineIndex			= -1;

	res = vkCreateGraphicsPipelines( vkDevice, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, VK_NULL_HANDLE, &vkPipeline );

	if (res != VK_SUCCESS)
	{
		printf( "Create graphics pipeline failed" );
		exit( 0 );
	}
}

void VKResources::CreateSurface( GLFWwindow* window )
{
	VkResult res = glfwCreateWindowSurface( vkInstance, window, nullptr, &vkSurface );

	if (res != VK_SUCCESS)
	{
		assert( 0 && "Vulkan error: create surface failed!" );
		glfwTerminate();
		std::exit( -1 );
	}
}

VkSurfaceFormatKHR VKResources::GetSwapChainFormat()
{
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR( selectedGPU, vkSurface, &formatCount, VK_NULL_HANDLE );
	std::vector<VkSurfaceFormatKHR> surfaceFormats( formatCount );
	vkGetPhysicalDeviceSurfaceFormatsKHR( selectedGPU, vkSurface, &formatCount, surfaceFormats.data() );

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

VkImageUsageFlags VKResources::GetSwapChainUsageFlags( VkSurfaceCapabilitiesKHR vkSurfaceCaps )
{
	// VK_IMAGE_USAGE_TRANSFER_DST image usage is not supported
	if (vkSurfaceCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
	{
		printf( "VK_IMAGE_USAGE_TRANSFER_DST image usage is not supported by the swap chain!\n" );

		return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	return static_cast<VkImageUsageFlags>(-1);
}

VkExtent2D VKResources::GetSwapChainExtent( VkSurfaceCapabilitiesKHR vkSurfaceCaps )
{
	if (vkSurfaceCaps.currentExtent.width == -1 || vkSurfaceCaps.currentExtent.height == -1)
	{
		VkExtent2D swapChainExtent = { surfaceWidth , surfaceHeight };

		if (swapChainExtent.width < vkSurfaceCaps.minImageExtent.width)
		{
			swapChainExtent.width = vkSurfaceCaps.minImageExtent.width;
		}

		if (swapChainExtent.height < vkSurfaceCaps.minImageExtent.height)
		{
			swapChainExtent.height = vkSurfaceCaps.minImageExtent.height;
		}
		
		if (swapChainExtent.width > vkSurfaceCaps.maxImageExtent.width)
		{
			swapChainExtent.width = vkSurfaceCaps.maxImageExtent.width;
		}

		if (swapChainExtent.height > vkSurfaceCaps.maxImageExtent.height)
		{
			swapChainExtent.height = vkSurfaceCaps.maxImageExtent.height;
		}

		return swapChainExtent;
	}
	else
	{
		return vkSurfaceCaps.currentExtent;
	}
}

VkSurfaceTransformFlagBitsKHR VKResources::GetSwapChainTransform( VkSurfaceCapabilitiesKHR vkSurfaceCaps )
{
	if (vkSurfaceCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		return vkSurfaceCaps.currentTransform;
	}
}

VkPresentModeKHR VKResources::GetSwapChainPresentMode()
{
	VkResult res;
	uint32_t presentModeCount = 0;

	res = vkGetPhysicalDeviceSurfacePresentModesKHR( selectedGPU, vkSurface, &presentModeCount, VK_NULL_HANDLE );
	assert( res == VK_SUCCESS && presentModeCount >= 1 );

	std::vector<VkPresentModeKHR> vkPresentModes( presentModeCount );
	res = vkGetPhysicalDeviceSurfacePresentModesKHR( selectedGPU, vkSurface, &presentModeCount, vkPresentModes.data() );
	assert( res == VK_SUCCESS );

	// Use mailbox as the best choise
	for (uint32_t i = 0; i < presentModeCount; i++)
	{
		if (vkPresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return VK_PRESENT_MODE_MAILBOX_KHR;
		}
	}

	// Use FIFO if mailbox is not supported
	for (uint32_t i = 0; i < presentModeCount; i++)
	{
		if (vkPresentModes[i] == VK_PRESENT_MODE_FIFO_KHR)
		{
			return VK_PRESENT_MODE_FIFO_KHR;
		}
	}

	return static_cast<VkPresentModeKHR>(-1);
}

void VKResources::CreateSwapChain( int windowHeight, int windowWidth )
{
	VkResult res;

	if (vkDevice != VK_NULL_HANDLE)
	{
		vkDeviceWaitIdle( vkDevice );
	}

	surfaceWidth = windowWidth;
	surfaceHeight = windowHeight;

	// Get surface capabilities
	VkSurfaceCapabilitiesKHR vkSurfaceCaps = {};
	res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR( selectedGPU, vkSurface, &vkSurfaceCaps );

	if (res != VK_SUCCESS)
	{
		assert( 0 && "Vulken error: get physical device surface cap failed!" );
		glfwTerminate();
		std::exit( -1 );
	}

	// Get image count for swap chain
	assert( vkSurfaceCaps.maxImageCount >= 1 );

	uint32_t desiredNumberOfImages = vkSurfaceCaps.minImageCount + 1;

	if (vkSurfaceCaps.maxImageCount > 0 && desiredNumberOfImages > vkSurfaceCaps.maxImageCount)
	{
		desiredNumberOfImages = vkSurfaceCaps.maxImageCount;
	}

	// Get image format and color space
	vkDesiredFormat = GetSwapChainFormat();
	// Get swap chain extent
	swapChainExtent = GetSwapChainExtent( vkSurfaceCaps );
	// Get swap chain image usage flags
	imageUsageFlags = GetSwapChainUsageFlags( vkSurfaceCaps );
	// Get swap chain transform
	swapChainTransform = GetSwapChainTransform( vkSurfaceCaps );
	// Get present modes
	presentMode = GetSwapChainPresentMode();
	VkSwapchainKHR oldSwapChain = vkSwapChain;

	// Create swap chain
	VkSwapchainCreateInfoKHR vkSwapChainCreateInfo		= {};
	vkSwapChainCreateInfo.sType							= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	vkSwapChainCreateInfo.surface						= vkSurface;
	vkSwapChainCreateInfo.minImageCount					= desiredNumberOfImages;
	vkSwapChainCreateInfo.imageFormat					= vkDesiredFormat.format;
	vkSwapChainCreateInfo.imageColorSpace				= vkDesiredFormat.colorSpace;
	vkSwapChainCreateInfo.imageExtent					= swapChainExtent;
	vkSwapChainCreateInfo.presentMode					= presentMode;
	vkSwapChainCreateInfo.imageArrayLayers				= 1;
	vkSwapChainCreateInfo.imageUsage					= imageUsageFlags;
	vkSwapChainCreateInfo.imageSharingMode				= VK_SHARING_MODE_EXCLUSIVE;
	vkSwapChainCreateInfo.queueFamilyIndexCount			= 0;
	vkSwapChainCreateInfo.pQueueFamilyIndices			= VK_NULL_HANDLE;
	vkSwapChainCreateInfo.preTransform					= swapChainTransform;
	vkSwapChainCreateInfo.compositeAlpha				= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	vkSwapChainCreateInfo.clipped						= VK_TRUE;
	vkSwapChainCreateInfo.oldSwapchain					= oldSwapChain;

	vkCreateSwapchainKHR( vkDevice, &vkSwapChainCreateInfo, VK_NULL_HANDLE, &vkSwapChain );

	if (oldSwapChain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR( vkDevice, oldSwapChain, VK_NULL_HANDLE );
	}

	res = vkGetSwapchainImagesKHR( vkDevice, vkSwapChain, &imageCount, VK_NULL_HANDLE );
	assert( res == VK_SUCCESS );

	printf( "Total image count: %d", imageCount );

	frameResources.resize( commandBufferCount );
	vkImages.resize( commandBufferCount );
	vkImageViews.resize( commandBufferCount );
	//vkFramebuffers.resize( commandBufferCount );

	res = vkGetSwapchainImagesKHR( vkDevice, vkSwapChain, &commandBufferCount, vkImages.data() );
	assert( res == VK_SUCCESS );
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

void VKResources::DestroyFrameResources()
{
	for (uint32_t i = 0; i < commandBufferCount; i++)
	{
		// Destroy frame buffer
		if (frameResources[i].frameBuffer != VK_NULL_HANDLE)
		{
			vkDestroyFramebuffer( vkDevice, frameResources[i].frameBuffer, VK_NULL_HANDLE );
		}

		// Free command buffer
		if (frameResources[i].commandBuffer != VK_NULL_HANDLE)
		{
			vkFreeCommandBuffers(
				vkDevice,
				vkCommandPool,
				1,
				&frameResources[i].commandBuffer
			);
		}

		// Destroy semaphores and fences
		if (frameResources[i].imageAvailableSemaphore != VK_NULL_HANDLE)
		{
			vkDestroySemaphore( vkDevice, frameResources[i].imageAvailableSemaphore, VK_NULL_HANDLE );
			vkDestroySemaphore( vkDevice, frameResources[i].renderingFinishedSemaphore, VK_NULL_HANDLE );
		}

		if (frameResources[i].fence != VK_NULL_HANDLE)
		{
			vkDestroyFence( vkDevice, frameResources[i].fence, VK_NULL_HANDLE );
		}
	}

	frameResources.clear();

	if (vkCommandPool != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool( vkDevice, vkCommandPool, VK_NULL_HANDLE );
		vkCommandPool = VK_NULL_HANDLE;
	}
}

void VKResources::DestroyVKInstance()
{
	vkDestroyInstance( vkInstance, VK_NULL_HANDLE );
	vkInstance = VK_NULL_HANDLE;
}

void VKResources::DestroyDevice()
{
	vkDestroyDevice( vkDevice, VK_NULL_HANDLE );
	vkDevice = VK_NULL_HANDLE;
}

void VKResources::DestroyPipeline()
{
	if (vkPipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline( vkDevice, vkPipeline, VK_NULL_HANDLE );
		vkPipeline = VK_NULL_HANDLE;
	}
	
	if (vkRenderPass != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass( vkDevice, vkRenderPass, VK_NULL_HANDLE );
		vkRenderPass = VK_NULL_HANDLE;
	}
}

void VKResources::DestroySwapChain()
{
	// Clear all images and image views
	for (uint32_t i = 0; i < commandBufferCount; i++)
	{
		vkDestroyImageView( vkDevice, vkImageViews[i], VK_NULL_HANDLE );
		vkDestroyImage( vkDevice, vkImages[i], VK_NULL_HANDLE );
	}

	vkImageViews.clear();
	vkImages.clear();

	vkDestroySwapchainKHR( vkDevice, vkSwapChain, VK_NULL_HANDLE );
	vkSwapChain = VK_NULL_HANDLE;
}

void VKResources::DestroyCommandPool()
{
	for (uint32_t i = 0; i < frameResources.size(); i++)
	{
		if ((frameResources[i].commandBuffer != VK_NULL_HANDLE))
		{
			vkFreeCommandBuffers(
				vkDevice,
				vkCommandPool,
				1,
				&frameResources[i].commandBuffer
			);
		}
	}

	if (vkCommandPool != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool( vkDevice, vkCommandPool, VK_NULL_HANDLE );
		vkCommandPool = VK_NULL_HANDLE;
	}
}

void VKResources::DestroySurface()
{
	vkDestroySurfaceKHR( vkInstance, vkSurface, VK_NULL_HANDLE );
	vkSurface = VK_NULL_HANDLE;
}

VkInstance VKResources::GetInstance()
{
	return vkInstance;
}

VkSurfaceKHR VKResources::GetSurface()
{
	return vkSurface;
}
