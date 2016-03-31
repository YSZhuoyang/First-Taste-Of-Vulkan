#pragma once

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing fla



class ModelLoader
{
public:
	ModelLoader();
	~ModelLoader();

	bool LoadMesh( const std::string Filename );

private:
	Assimp::Importer				importer;
};
