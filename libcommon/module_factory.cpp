#include "libcommon/module_factory.h"


extern "C" _declspec( dllexport ) ModuleDictionary *GetGlobalModuleDict()
{
    static ModuleDictionary s_dict;
    return &s_dict;
}