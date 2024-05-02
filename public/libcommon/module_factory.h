#pragma once

#include "common_stl.h"

class IModule;

template<class T>
concept DerivedModule = std::is_base_of<IModule, T>::value;

namespace Modules
{
    template<DerivedModule T>
    struct DeclareModule;
}

// Only a single instance of ModuleDictionary can exist within a library.
class ModuleDictionary
{

public:
    void Add(String name, IModule *module)
    {
        Dict<String, IModule *> *interfaces = &LocalInterfacesDict;
        if ( UseGlobal ) interfaces = GlobalInterfacesDict;

        if ( interfaces->size() == 0 || interfaces->find(name) == interfaces->end())
        {
            (*interfaces)[name] = module;
        }
    }

    IModule *Find( const char *name )
    {
        Dict<String, IModule *> *interfaces = &LocalInterfacesDict;
        if ( UseGlobal ) interfaces = GlobalInterfacesDict;

        auto iter = interfaces->find( name );
        if ( iter != interfaces->end() )
        {
            return iter->second;
        }

        return nullptr;
    }

    void Merge( ModuleDictionary *other )
    {
        for ( auto module : other->LocalInterfacesDict )
        {
            Add( module.first, module.second );
        }

        other->UseGlobal = true;

        if( UseGlobal )
            other->GlobalInterfacesDict = GlobalInterfacesDict;
        else
            other->GlobalInterfacesDict = &LocalInterfacesDict;
    }

private:
    Dict<String, IModule *> *GlobalInterfacesDict;
    Dict<String, IModule *> LocalInterfacesDict;
    bool UseGlobal = false;
};

class IModule
{
public:
    IModule( const String classname ) { Classname = classname; }

    virtual void *GetInterface() = 0;

    String Classname = "UNDEFINED MODULE CLASS";
};

extern "C" _declspec(dllexport) ModuleDictionary *GetGlobalModuleDict();
using GetGlobalModuleDict_Function = decltype( GetGlobalModuleDict );

namespace Modules
{
    template<DerivedModule T>
    struct DeclareModule
    {
        T *module_self = nullptr;
        DeclareModule()
        {
            module_self = new T();
            GetGlobalModuleDict()->Add( module_self->Classname.c_str(), module_self);
        }

        T *operator->()
        {
            return module_self;
        }

        T &operator*()
        {
            return *module_self;
        }

        T *operator&()
        {
            return module_self;
        }
    };
}