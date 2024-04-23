#pragma once

class IRenderSystem
{
public:

	// Create graphics API instance here (if required) and select device
	virtual bool CreateRenderer() = 0;

	// Attach to a window, creates surface
	virtual void AttachWindow( void *window_handle, int w, int h ) = 0;

};