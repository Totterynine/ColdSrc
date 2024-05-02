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
		LibHandle lib = SDL_LoadObject( ( name + ".dll" ).c_str() );

		auto OtherModuleDict = Modules::GetLibraryModuleDict( lib );
		if ( !OtherModuleDict )
			return nullptr;

		GetGlobalModuleDict()->Merge( OtherModuleDict );

		return lib;
	}

	ModuleDictionary *GetLibraryModuleDict( LibHandle libHandle )
	{
		GetGlobalModuleDict_Function *getFunc = FindGetFunction( libHandle );
		if( !getFunc )
			return nullptr;

		return getFunc();
	}

}