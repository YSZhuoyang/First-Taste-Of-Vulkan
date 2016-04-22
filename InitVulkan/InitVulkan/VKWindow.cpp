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

	//glfwGetFramebufferSize( window, &windowWidth, &windowHeight );
	//glfwShowWindow( window );

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

int GLFWWindowResources::VKWindow::GetHeight()
{
	return windowHeight;
}

int GLFWWindowResources::VKWindow::GetWidth()
{
	return windowWidth;
}

GLFWwindow* GLFWWindowResources::VKWindow::GetWindowInstance()
{
	return window;
}
