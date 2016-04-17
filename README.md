# First-Taste-Of-Vulkan

This project uses Assimp as model loader, and GLFW3 for handling window creation.

Steps setup Vulkan environment:

1. Install VulkanSDK (current version: 1.0.8.0)

2. Go to 'Project properties -> Configuration Properties -> VC++ Directory', Added vulkan include folder path to include directories, add Bin32 path (for 32 bit mode, Bin path for 64 bit mode) to lib directories.

3. Switch to 64 debug mode, and repeat the step 2.


Some steps to get the project AssimpSetup up and running:

1. Open it with Visual Studio 2015. In 32 Debug mode, go to 'Project properties -> Configuration Properties -> Debugging', copy and paste the following path into 'Environment' field:
      PATH=%PATH%;$(ProjectDir)\..\..\AssimpBinary\bin32$(LocalDebuggerEnvironment)

2. Switch to 64 debug mode, and repeat the step 1 (bin32 for 32 bit mode, bin for 64 bit mode).

It is recommended to go to Assimp and GLFW websites on Github to clone the latest master branch and build the library yourself using CMake.
