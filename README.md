## First-Taste-Of-Vulkan

This project uses Assimp as model loader, and GLFW3 for window creation.

Steps to setup Vulkan environment:
* Install VulkanSDK (current version: 1.0.13.0)
* Go to 'Project properties -> Configuration Properties -> VC++ Directory', Added vulkan 'include' folder path to 'include directories', add 'Bin32' folder path in 32 bit mode, add 'Bin' folder path in 64 bit mode to lib directories.
* Switch to 64 debug mode, and repeat the step 2.


Steps to get project AssimpSetup up and running:
* Open it with Visual Studio 2015. In 32 Debug mode, go to 'Project properties -> Configuration Properties -> Debugging', copy and paste the following path into 'Environment' field:

            PATH=%PATH%;$(ProjectDir)\..\..\AssimpBinary\bin32$(LocalDebuggerEnvironment)

* Switch to 64 debug mode, and repeat the step 1 (bin32 for 32 bit mode, bin for 64 bit mode).

It is recommended to go to Assimp and GLFW websites on Github to clone the latest master branch and build the library yourself using CMake.
