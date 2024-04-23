#pragma once

// Custom application that's defined and created by User and handled by appframework.

class IApplication
{
public:
    virtual const char *GetAppName() = 0;

    // Called when app is first started and shutdown.
    virtual bool Execute() = 0;
    virtual void Shutdown() = 0;

    // Here you can process Device inputs before frame begins.
    virtual bool ProccessWindowEvents() = 0;

    // Run the application logic before frame begins.
    virtual void Simulate(float dt) = 0;

    // Runs the process of rendering the frame, then it presenting on selected surface.
    virtual void Frame() = 0;
};