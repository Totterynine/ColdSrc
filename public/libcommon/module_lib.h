#pragma once

#include "common_stl.h"
#include "libcommon/module_factory.h"

namespace Modules {

	using LibHandle = void *;

	LibHandle LoadLib( String name );

	ModuleDictionary *GetLibraryModuleDict( LibHandle libHandle );

	template<class T>
	T *FindModule()
	{
		return GetGlobalModuleDict()->FindModule<T>();
	}

}