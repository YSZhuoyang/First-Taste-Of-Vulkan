#include "Renderer.h"
#include "DataStructures.h"


using namespace DataStructures;

Renderer::Renderer()
{
	vkResources = new VKResources();
	vkWindow = new VKWindow();

	vkWindow->CreateWindow();
	vkResources->InitVKInstance();
	vkResources->InitDevice();
	vkResources->CreateSurface( vkWindow->GetWindowInstance() );
	vkResources->CreateSwapChain( vkWindow->GetHeight(), vkWindow->GetWidth() );
	vkResources->CreateCommandPool();
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

bool Renderer::OnWindowSizeChanged()
{


	return true;
}

void Renderer::Update()
{
	while (!glfwWindowShouldClose( vkWindow->GetWindowInstance() ))
	{
		Render();
		glfwPollEvents();
	}

	glfwTerminate();
	exit( EXIT_SUCCESS );
}

void Renderer::Render()
{
	uint32_t imageIndex = vkResources->AcquireImageIndex( vkWindow );
	vkResources->SubmitBuffers( imageIndex, vkWindow );
}
