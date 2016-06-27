#pragma once

#include <vulkan\vulkan.h>
#include <GLFW\glfw3.h>
#include <cstdlib>
#include <assert.h>
#include <vector>

#include "DataStructures.h"
#include "VKResources.h"
#include "VKWindow.h"


using namespace DataStructures;
using namespace VulkanResources;
using namespace GLFWWindowResources;

class Renderer
{
public:
	Renderer();
	~Renderer();

	void Init();
	void Stop();
	void Run();
	bool IsRunning();

private:
	void Render();
	void Update();

	VKResources*					vkResources				= nullptr;
	VKWindow*						vkWindow				= nullptr;
	bool							isRunning				= true;
};
