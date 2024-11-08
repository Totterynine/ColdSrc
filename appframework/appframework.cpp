#include "appframework.h"
#include "libcommon/module_lib.h"

const String DefaultApp = "game";

void AppFramework::Execute(const Span<String> &args)
{
	CurrentApp = DefaultApp;

	for ( Span<String>::iterator iter = args.begin(); iter != args.end(); ++iter )
	{
		String &arg = *iter;
		if ( arg == "-game" )
		{
			iter++;
			if ( iter != args.end() )
				CurrentApp = *iter;
		}
	}

	Modules::LibHandle appLib = Modules::LoadLib( CurrentApp );
	Modules::LibHandle rendersysLib = Modules::LoadLib( "rendersystem" );

	if ( !appLib || !rendersysLib )
		return;

	TheApp = Modules::FindModule<IApplication>();

	if ( !TheApp->Execute() )
		exit( 0 );

	while ( TheApp->ProccessWindowEvents() )
	{
		TheApp->Simulate( 1.0f / 60.0f );

		TheApp->Frame();
	}

	TheApp->Shutdown();
}