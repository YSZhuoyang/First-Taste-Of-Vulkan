
#include "Renderer.h"

int main()
{
	Renderer r;

	printf( "Renderer initialized successfully" );

	while (r.IsRunning())
	{
		r.Update();
	}

	return 0;
}
