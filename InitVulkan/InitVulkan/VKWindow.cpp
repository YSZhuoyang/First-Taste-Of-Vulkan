#include "VKWindow.h"

using namespace GLFWWindowResources;


static void windowSizeCallback( GLFWwindow* window, int width, int height )
{
	glfwSetWindowSize( window, width, height );
}

static void keyCallback( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose( window, GL_TRUE );
	}
}

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

void VKWindow::Resize()
{
	
}

void VKWindow::Close()
{

}
