#include "VKWindow.h"


using namespace GLFWWindowResources;


VKWindow::VKWindow()
{
	
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
}

void VKWindow::DestroyWindow()
{
	glfwDestroyWindow( window );
	glfwTerminate();
	window = nullptr;
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

void VKWindow::Resize()
{

}

void VKWindow::Close()
{

}
