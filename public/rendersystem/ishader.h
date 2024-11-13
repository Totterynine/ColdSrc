#pragma once
#include "rendersystem/rendersystem_types.h"

class IDescriptorLayout
{
public:

};

class IDescriptorSet
{
public:
    virtual void Init(IDescriptorLayout* layout) = 0;

    virtual void BindImage(uint32_t binding, HImageView img) = 0;

    virtual void Update() = 0;
};

class IShader
{
public:

    virtual ShaderType GetType() = 0;

    // After setting the modules, the shader will take ownership of them and destroy them with itself
    // TODO: Need a shader module cache that will take ownership of shader modules
    virtual void SetVertexModule(HShader csModule) = 0;
    virtual void SetFragmentModule(HShader csModule) = 0;
    virtual void SetComputeModule(HShader csModule) = 0;

    // All of these must be set before building pipeline
    virtual void SetTopology(PrimitiveTopology topology) = 0;
    virtual void SetPolygonMode(PolygonMode polygonMode) = 0;
    virtual void SetCullMode(CullModeFlags cullFlags, PolygonWinding winding) = 0;

    virtual void BuildPipeline(IDescriptorLayout* layout) = 0;

};