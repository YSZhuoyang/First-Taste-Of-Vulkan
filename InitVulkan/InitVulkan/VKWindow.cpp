#include "VKWindow.h"

using namespace GLFWWindowResources;


void VKWindow::KeyPressEvent( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose( window, GL_TRUE );
	}
}

void VKWindow::WindowResizeEvent( GLFWwindow* window, int width, int height )
{
	//glfwGetWindowSize( window, &windowWidth, &windowHeight );

	//glfwSetWindowSize( window, width, height );
	vkResources->OnWindowSizeChanged( window, width, height );
	
	windowWidth = width;
	windowHeight = height;
}

VKWindow::VKWindow( VKResources* vkResources )
{
	this->vkResources = vkResources;
}

VKWindow::~VKWindow()
{
	DestroyWindow();
}

void VKWindow::CreateWindow()
{
	// Tell GLFW not to create OpenGL context with a window
	glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
	window = glfwCreateWindow( windowWidth, windowHeight, "First taste of Vulkan", nullptr, nullptr );
	glfwSetWindowUserPointer( window, this );
	
	auto keyCallback = [] ( GLFWwindow* window, int key, int scancode, int action, int mods )
	{
		static_cast<VKWindow*>(glfwGetWindowUserPointer( window ))->KeyPressEvent( window, key, scancode, action, mods );
	};

	auto windowSizeCallback = [] ( GLFWwindow* window, int width, int height )
	{
		static_cast<VKWindow*>(glfwGetWindowUserPointer( window ))->WindowResizeEvent( window, width, height );
	};

	glfwSetKeyCallback( window, keyCallback );
	glfwSetWindowSizeCallback( window, windowSizeCallback );
}

void VKWindow::DestroyWindow()
{
	glfwDestroyWindow( window );
	glfwTerminate();
	window = nullptr;
}

void VKWindow::SetWidth( int width )
{
	windowWidth = width;
}

void VKWindow::SetHeight( int height )
{
	windowHeight = height;
}

int VKWindow::GetHeight()
{
	return windowHeight;
}

int VKWindow::GetWidth()
{
	return windowWidth;
}

GLFWwindow* VKWindow::GetWindowInstance()
{
	return window;
}
