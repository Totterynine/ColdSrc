#pragma once

struct BlendState
{
    bool enable;
    float src_factor, dst_factor;
};

struct DepthStencilState
{
    bool enable;
    float depth_bias;
};

struct RasterizerState
{
    bool enable;
    int cull_mode;
};

struct Viewport
{
    int x, y, w, h;
};

struct ScissorRectangle
{
    int x, y, w, h;
};

struct ColorFloat
{
    float r, g, b, a;
};

class IRenderTarget;
class IVertexBuffer;
class IIndexBuffer;

class IShader
{
public:

};

class IRenderSystem
{
public:
    // Create the rendering system
    virtual bool Create() = 0;

    // Attach the rendering system to a window
    virtual void AttachWindow(void *window_handle, int w, int h) = 0;

    // Begin rendering
    virtual void BeginRendering() = 0;

    // End rendering
    virtual void EndRendering() = 0;

    virtual void SetClearColor(ColorFloat &color) = 0;
    virtual void ClearColor() = 0;

    // Set the render target
    // Set nullptr to clear
    virtual void SetRenderTarget(IRenderTarget *target) = 0;

    // Set the viewport
    virtual void SetViewport(Viewport settings) = 0;

    // Set the scissor rectangle
    virtual void SetScissorRectangle(ScissorRectangle settings) = 0;

    // Set the current shader to render the mesh
    // Set to nullptr to clear
    virtual void SetShader(IShader *shader) = 0;

    // Set the vertex buffer
    virtual void SetVertexBuffer(IVertexBuffer *buffer) = 0;

    // Set the index buffer
    virtual void SetIndexBuffer(IIndexBuffer *buffer) = 0;

    // Draw a primitive
    virtual void DrawPrimitive(int primitive_type, int vertex_count) = 0;

    // Draw indexed primitives
    virtual void DrawIndexedPrimitives(int primitive_type, int index_count) = 0;

    // Present the render target to surface
    virtual void Present() = 0;

    // Destroy the rendering system
    virtual void Destroy() = 0;

    // Set the blend state
    virtual void SetBlendState(BlendState settings) = 0;

    // Set the depth stencil state
    virtual void SetDepthStencilState(DepthStencilState settings) = 0;

    // Set the rasterizer state
    virtual void SetRasterizerState(RasterizerState settings) = 0;
};