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
        if ( InterfacesDict.size() == 0 || InterfacesDict.find(name) == InterfacesDict.end())
        {
            InterfacesDict[name] = module;
        }
    }

    IModule *Find( const char *name )
    {
        auto iter = InterfacesDict.find( name );
        if ( iter != InterfacesDict.end() )
        {
            return iter->second;
        }

        return nullptr;
    }

    void Merge( ModuleDictionary *other )
    {
        for ( auto module : other->InterfacesDict )
        {
            Add( module.first, module.second );
        }
    }

private:
    Dict<String, IModule *> InterfacesDict;
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