#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>
#include <assert.h>
#include <cstdlib>


namespace GLFWWindowResources
{
	class VKWindow
	{
	public:
		VKWindow();
		~VKWindow();

		void CreateWindow( VkInstance vkInstance, VkSurfaceKHR vkSurface );
		void DestroyWindow();
		int GetHeight();
		int GetWidth();
		GLFWwindow* GetWindowInstance();

	private:
		GLFWwindow*						window					= nullptr;
		int								windowHeight			= 640;
		int								windowWidth				= 960;
	};
}
