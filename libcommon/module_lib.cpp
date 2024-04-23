#include "libcommon/module_lib.h"

#include "SDL.h"

static GetGlobalModuleDict_Function *FindGetFunction( Modules::LibHandle libHandle )
{
	GetGlobalModuleDict_Function *hFunc = reinterpret_cast<GetGlobalModuleDict_Function *>( SDL_LoadFunction( libHandle, "GetGlobalModuleDict" ) );

	return hFunc;
}

namespace Modules {

	LibHandle LoadLib( String name )
	{
		return SDL_LoadObject( (name + ".dll" ).c_str());
	}

	ModuleDictionary *GetLibraryModuleDict( LibHandle libHandle )
	{
		GetGlobalModuleDict_Function *getFunc = FindGetFunction( libHandle );
		if( !getFunc )
			return nullptr;

		return getFunc();
	}

}