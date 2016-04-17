#include "Renderer.h"
#include "DataStructures.h"


using namespace DataStructures;

Renderer::Renderer()
{
	vkResources = new VKResources();
	vkWindow = new VKWindow();

	vkResources->InitVKInstance();
	vkResources->InitDevice();
	vkResources->CreateSwapChain( vkWindow->GetHeight(), vkWindow->GetWidth() );
	vkResources->CreateCommandPool();

	vkWindow->CreateWindow( vkResources->GetInstance(), vkResources->GetSurface() );
}

Renderer::~Renderer()
{
	delete vkResources;
	vkResources = nullptr;

	delete vkWindow;
	vkWindow = nullptr;
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
	Render();
}

void Renderer::Render()
{
	//glfwSwapBuffers( window );
}
