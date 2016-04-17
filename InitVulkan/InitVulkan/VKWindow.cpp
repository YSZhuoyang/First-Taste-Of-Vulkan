#include "VKWindow.h"


using namespace GLFWWindowResources;


VKWindow::VKWindow()
{
	
}


VKWindow::~VKWindow()
{
	DestroyWindow();
}

void VKWindow::CreateWindow( VkInstance vkInstance, VkSurfaceKHR vkSurface )
{
	// Tell GLFW not to create OpenGL context with a window
	glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
	window = glfwCreateWindow( windowWidth, windowHeight, "First taste of Vulkan", nullptr, nullptr );

	VkResult res = glfwCreateWindowSurface( vkInstance, window, nullptr, &vkSurface );

	glfwGetFramebufferSize( window, &windowWidth, &windowHeight );
	glfwShowWindow( window );
	//glfwMakeContextCurrent( window );

	if (res != VK_SUCCESS)
	{
		assert( 0 && "Vulkan error: create surface failed!" );
		glfwTerminate();
		std::exit( -1 );
	}
}

void VKWindow::DestroyWindow()
{
	glfwDestroyWindow( window );
	glfwTerminate();
	window = nullptr;
}

GLFWwindow* GLFWWindowResources::VKWindow::GetWindowInstance()
{
	return window;
}
