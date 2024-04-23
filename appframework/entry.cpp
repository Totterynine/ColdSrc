#include "appframework.h"
#include "SDL.h"
#include "SDL_main.h"

int main(int argc, char *argv[])
{
    AppFramework framework = AppFramework();

    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "Couldn't initialize SDL: %s\n", SDL_GetError() );
        exit( 1 );
    }

    Array<String> args;
    for (int i = 0; i < argc; ++i)
        args.push_back( String(argv[i]));

    framework.Execute(Span(args.begin(), args.end()));

    return 0;
}