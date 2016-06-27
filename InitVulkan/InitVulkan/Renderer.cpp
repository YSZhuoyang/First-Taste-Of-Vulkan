#include "Renderer.h"
#include "DataStructures.h"


using namespace DataStructures;

Renderer::Renderer()
{
	vkResources = new VKResources();
	vkWindow = new VKWindow( vkResources );
}

Renderer::~Renderer()
{
	delete vkResources;
	vkResources = nullptr;

	delete vkWindow;
	vkWindow = nullptr;
}

void Renderer::Init()
{
	vkWindow->CreateWindow();
	vkResources->InitVKInstance();
	vkResources->InitDevice();
	vkResources->CreateSurface( vkWindow->GetWindowInstance() );
	vkResources->CreateSwapChain( vkWindow->GetHeight(), vkWindow->GetWidth() );
	vkResources->CreateCommandPool();
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

}

void Renderer::Run()
{
	while (!glfwWindowShouldClose( vkWindow->GetWindowInstance() ))
	{
		Update();
		Render();
		glfwPollEvents();
	}

	glfwTerminate();
	exit( EXIT_SUCCESS );
}

void Renderer::Render()
{
	uint32_t imageIndex = vkResources->AcquireImageIndex( vkWindow->GetWindowInstance() );
	vkResources->SubmitBuffers( imageIndex, vkWindow->GetWindowInstance() );
}
