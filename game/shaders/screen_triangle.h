#pragma once
#include "baseshader.h"

class ShaderScreenTriangle : public BaseShader
{
protected:

    virtual void Snapshot()
    {
        SetVertexShader("triangle_vs61.spv");
        SetFragmentShader("triangle_ps61.spv");

        SetTopology(PrimitiveTopology::Triangles);
        SetPolygonMode(PolygonMode::Fill);
        SetCullMode(CullModeFlags::None, PolygonWinding::CounterClockwise);
    }
};