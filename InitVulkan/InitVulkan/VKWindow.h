#pragma once

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

		void CreateWindow();
		void DestroyWindow();
		int GetHeight();
		int GetWidth();
		GLFWwindow* GetWindowInstance();

		// Callback functions
		void Resize();
		void Close();


	private:
		GLFWwindow*						window					= nullptr;
		int								windowHeight			= 640;
		int								windowWidth				= 960;
	};
}
