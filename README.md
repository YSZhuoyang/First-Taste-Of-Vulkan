# First-Taste-Of-Vulkan

This project uses Assimp as model loader, and GLFW3 for handling window creation.

Some steps to get the project AssimpSetup up and running:

1. Open it with Visual Studio 2015. In 32 Debug mode, go to Project properties -> Configuration Properties -> Debugging, copy and paste the following path into 'Environment' field:
   PATH=%PATH%;$(ProjectDir)\..\..\AssimpBinary\bin32$(LocalDebuggerEnvironment)

2. Switch to 64 debug mode, and repeat the step 1.

It is recommended to go to Assimp and GLFW websites on Github to clone the latest master branch and build the library yourself using CMake.
