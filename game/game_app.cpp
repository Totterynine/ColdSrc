#include "appframework/iapplication.h"
#include "game_globals.h"
#include "libcommon/module_lib.h"
#include "SDL.h"

#include <iostream>

IRenderSystem *rendersys = nullptr;

class GameApp : public IApplication, public IModule
{
public:

	GameApp() : IModule("GameApp") {}

    virtual void *GetInterface() { return static_cast<IApplication *>( this ); }

public:

    virtual const char *GetAppName() { return "Test Game"; };

    // Called when app is first started and shutdown.
    virtual bool Execute() 
    {
        if ( !LoadDependencies() )
            return false;

        SDL_Window *window = SDL_CreateWindow( GetAppName(), 1280, 720, 0 );

        if ( !window )
        {
            std::cout << SDL_GetError();
        }

        void *hwnd = SDL_GetProperty( SDL_GetWindowProperties( window ), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL );

        if ( !rendersys->CreateRenderer() )
            return false;

        rendersys->AttachWindow( hwnd );

        return true;
    }
    virtual void Shutdown()
    {

    }

    // Here you can process Device inputs before frame begins.
    virtual bool ProccessWindowEvents()
    {
        bool resume = true;

        //Event handler
        SDL_Event e;

        //Handle events on queue
        while ( SDL_PollEvent( &e ) != 0 )
        {
            //User requests quit
            if ( e.type == SDL_EVENT_QUIT )
            {
                resume = false;
            }
        }

        return resume;
    }

    // Run the application logic before frame begins.
    virtual void Simulate( float dt )
    {

    }

    // Runs the process of rendering the frame, then it presenting on selected surface.
    virtual void Frame()
    {

    }

private:

    bool LoadDependencies()
    {
        Modules::LibHandle rendersysLib = Modules::LoadLib( "rendersystem" );

        if ( !rendersysLib )
            return false;

        auto GetModuleDict = Modules::GetLibraryModuleDict( rendersysLib );

        if ( !GetModuleDict )
            return false;

        IModule *rendersysModule = GetModuleDict->Find( "RenderSystem" );

        if ( !rendersysModule )
            return false;

        rendersys = reinterpret_cast<IRenderSystem *>( rendersysModule->GetInterface() );

        return true;
    }

};

static Modules::DeclareModule<GameApp> game_module;