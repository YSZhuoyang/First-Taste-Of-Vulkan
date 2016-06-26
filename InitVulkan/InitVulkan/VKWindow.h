#pragma once

#include <GLFW\glfw3.h>
#include <assert.h>
#include <cstdlib>

#include "VKResources.h"

using namespace VulkanResources;


namespace GLFWWindowResources
{
	class VKWindow
	{
	public:
		VKWindow( VKResources* vkResources );
		~VKWindow();

		// Event handling 
		void KeyPressEvent( GLFWwindow *window, int key, int scancode, int action, int mods );
		void WindowResizeEvent( GLFWwindow* window, int width, int height );

		void CreateWindow();
		void DestroyWindow();
		void SetWidth( int width );
		void SetHeight( int height );
		int GetHeight();
		int GetWidth();
		GLFWwindow* GetWindowInstance();


	private:
		GLFWwindow*						window					= nullptr;
		VKResources*					vkResources				= nullptr;
		int								windowHeight			= 640;
		int								windowWidth				= 960;
	};
}
