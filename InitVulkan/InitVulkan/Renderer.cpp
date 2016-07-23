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
	vkResources->CreateRenderPass();
	vkResources->CreateFrameBuffers();
	vkResources->CreatePipeline();
	UploadVertexData();
	vkResources->CreateCommandPool();
	vkResources->CreateSemaphores();
	vkResources->CreateFences();
}

void Renderer::Stop()
{
	isRunning = false;
}

bool Renderer::IsRunning()
{
	return isRunning;
}

void Renderer::UploadVertexData()
{
	Vertex vertexData[] = {
		{ -0.7f, -0.7f, 0.0f, 1.0f },
		{ -0.7f, 0.7f, 0.0f, 1.0f },
		{ 0.7f, -0.7f, 0.0f, 1.0f }
	};

	vkResources->SetupVertexBuffer( vertexData );
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
	vkResources->AcquireAndSubmitFrame( vkWindow->GetWindowInstance() );
}
