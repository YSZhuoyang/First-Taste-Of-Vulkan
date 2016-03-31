#include "ModelLoader.h"
#include <stdlib.h>


ModelLoader::ModelLoader()
{
	
}


ModelLoader::~ModelLoader()
{

}

bool ModelLoader::LoadMesh( const std::string Filename )
{
	bool Ret = false;

	const aiScene* pScene = importer.ReadFile( Filename.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs );

	if (pScene)
	{
		//Ret = InitFromScene( pScene, Filename );
	}
	else
	{
		printf( "Error parsing '%s': '%s'\n", Filename.c_str(), importer.GetErrorString() );
	}

	return Ret;
}
