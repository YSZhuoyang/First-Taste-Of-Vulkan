#include "Renderer.h"
#include "DataStructures.h"


using namespace DataStructures;

Renderer::Renderer()
{
	vkResources = new VKResources();

	vkResources->InitVKInstance();
	vkResources->InitDevice();
	vkResources->CreateWindow();
	vkResources->CreateSwapChain();
	vkResources->CreateCommandPool();
}

Renderer::~Renderer()
{
	delete vkResources;
	vkResources = nullptr;
}

void Renderer::Stop()
{
	isRunning = false;
}

bool Renderer::IsRunning()
{
	return isRunning;
}

void Renderer::Update()
{
	while (!glfwWindowShouldClose( vkResources->GetWindowInstance() ))
	{
		glfwPollEvents();
		Render();
	}
}

void Renderer::Render()
{
	
}
