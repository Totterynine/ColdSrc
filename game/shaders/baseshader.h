#pragma once
#include "../game_globals.h"

class BaseShader
{
public:

    BaseShader()
    {
        internal_shader = rendersys->CreateShader();
    }

    void Initialize()
    {
        Snapshot();

        internal_shader->BuildPipeline(layout);
    }

    IShader *GetRenderShader()
    {
        return internal_shader;
    }

protected:

    virtual void Snapshot() = 0;

    void SetVertexShader(const char *shader)
    {
        HShader vsModule = rendersys->LoadShaderModule(shader);
        internal_shader->SetVertexModule(vsModule);
    }
    void SetFragmentShader(const char *shader)
    {
        HShader fsModule = rendersys->LoadShaderModule(shader);
        internal_shader->SetFragmentModule(fsModule);
    }
    void SetComputeShader(const char *shader)
    {
        HShader csModule = rendersys->LoadShaderModule(shader);
        internal_shader->SetComputeModule(csModule);
    }
    void SetTopology(PrimitiveTopology topology)
    {
        internal_shader->SetTopology(topology);
    }
    void SetPolygonMode(PolygonMode polygonMode)
    {
        internal_shader->SetPolygonMode(polygonMode);
    }
    void SetCullMode(CullModeFlags cullFlags, PolygonWinding winding)
    {
        internal_shader->SetCullMode(cullFlags, winding);
    }

    void SetDescriptorLayout(uint32_t numEntries, DescriptorLayoutEntry* entries)
    {
        layout = rendersys->BuildDescriptorLayout(numEntries, entries);
    }

private:

    IShader *internal_shader = nullptr;
    IDescriptorLayout *layout = nullptr;
};