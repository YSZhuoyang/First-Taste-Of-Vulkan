
#include "Renderer.h"

int main()
{
	Renderer r;

	printf( "finished" );

	while (r.IsRunning())
	{
		r.Update();
	}

	getchar();

	return 0;
}
