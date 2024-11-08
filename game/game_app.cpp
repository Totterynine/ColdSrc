#include "appframework/iapplication.h"
#include "game_globals.h"
#include "libcommon/module_lib.h"
#include "SDL.h"

#include <iostream>

IRenderSystem *rendersys = nullptr;

class GameApp : public IApplication
{
public:

    double CurrentTime = 0.0;

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

        if ( !rendersys->Create() )
            return false;

        rendersys->AttachWindow( &hwnd, 1280, 720 );

        // HDR Render target to support higher color values
        rendertarget = rendersys->CreateRenderTarget(ImageFormat::RGBA16F, 1280, 720);

        // Load shader module from file, get a new instance from rendersystem, give it our compute shader module
        HShader hardwareShader = rendersys->LoadShaderModule("circle_cs61.spv");
        circle_shader = rendersys->CreateShader();
        circle_shader->SetComputeModule(hardwareShader);

        // Build shader input layout
        DescriptorLayoutEntry layout[] =
        {
            {0, DescriptorType::Image, ShaderStage::Compute},
        };
        circle_shader_input_layout = rendersys->BuildDescriptorLayout(1, layout);

        // Build shader pipeline according to our input layout
        circle_shader->BuildPipeline(circle_shader_input_layout);

        // Create a new Descriptor set, bind our rendertarget into it, update for changes to take effect
        circle_shader_descriptor = rendersys->BuildDescriptorSet(circle_shader_input_layout);
        circle_shader_descriptor->BindImage(0, rendertarget->GetHardwareImageView());
        circle_shader_descriptor->Update();

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
            if (e.type == SDL_EVENT_WINDOW_MINIMIZED)
            {
                IsMinimized = true;
            }
            if (e.type == SDL_EVENT_WINDOW_SHOWN || e.type == SDL_EVENT_WINDOW_RESTORED)
            {
                IsMinimized = false;
            }
        }

        return resume;
    }

    // Run the application logic before rendering frame.
    virtual void Simulate( float dt )
    {
        CurrentTime += dt;
    }

    // Runs the process of rendering the frame, then presenting it on selected surface.
    virtual void Frame()
    {
        // In some GPU Drivers, the window surface extent is reduced to 0 when
        // minimized, which crashes the Vulkan swapchain creation
        if (IsMinimized)
        {
            return;
        }

        ColorFloat clearClr = { };
        clearClr.r = sin( CurrentTime / 2 ) + 1.0;
        clearClr.g = sin( CurrentTime + ( 3.1415926 / 2.0 ) ) + 1;
        clearClr.b = sin( CurrentTime + 3.1415926 ) + 1;
        clearClr.a = 1.0;

        clearClr.r /= 2.0;
        clearClr.g /= 2.0;
        clearClr.b /= 2.0;

        rendersys->BeginRendering();

        rendersys->SetViewport( { 0,0,1280,720 } );
        rendersys->SetRenderTarget(rendertarget);

        rendersys->SetClearColor( clearClr );
        rendersys->ClearColor();

        rendersys->BindShader(circle_shader, PipelineBindPoint::Compute);
        rendersys->BindDescriptorSet(circle_shader_descriptor, PipelineBindPoint::Compute);

        rendersys->Dispatch(1280 / 2, 720 / 2, 1);

        rendersys->CopyRenderTargetToBackBuffer();

        rendersys->EndRendering();
        rendersys->Present();

    }

private:

    bool LoadDependencies()
    {
        rendersys = Modules::FindModule< IRenderSystem >();

        if ( !rendersys )
            return false;

        return true;
    }

    IRenderTarget* rendertarget;
    IShader* circle_shader;
    IDescriptorSet* circle_shader_descriptor;
    IDescriptorLayout* circle_shader_input_layout;

    SDL_Window* Window;
    bool IsMinimized = false;
};

static Modules::DeclareModule<GameApp> game_module;