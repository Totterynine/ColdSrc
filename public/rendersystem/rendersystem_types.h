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
    unsigned int x, y, w, h;
};

struct ScissorRectangle
{
    int x, y, w, h;
};

struct ColorFloat
{
    float r, g, b, a;
};

enum class BufferFormat : short
{
    Null = 0,
    R8,
    R16,
    R32,

    R16F,
    R32F,

    RG8,
    RG16,
    RG32,

    RG16F,
    RG32F,

    RGB8,
    RGB16,
    RGB32,

    RGB16F,
    RGB32F,

    RGBA8,
    RGBA16,
    RGBA32,

    RGBA16F,
    RGBA32F,
};

// Hardware image handles
using HImage = void*;
using HImageView = void*;

// Hardware shader handle
using HShader = void*;

class IVertexBuffer;
class IIndexBuffer;

enum class ShaderType : unsigned char
{
    Null = 0,
    Graphics,
    Compute,
};

enum ShaderStage : unsigned char
{
    Null = 0,
    Vertex = 0x01,
    Pixel = 0x02,
    Compute = 0x04,
};

enum class DescriptorType : short
{
    ConstantBuffer = 0, // Equivalent to Uniform Buffer

    // 'Storage' is equivalent to Unordered Access View
    StorageBuffer,
    StorageImage,
};

enum class PipelineBindPoint : short
{
    Graphics = 0,
    Compute
};

struct DescriptorLayoutEntry
{
    uint32_t Binding;
    DescriptorType Type;
    ShaderStage Stage;
};

enum PrimitiveTopology : int
{
    Points = 0,
    Lines,
    LineStrip,
    Triangles
};

enum PolygonMode : int
{
    Fill = 0,
    Line,
    Point,
};

enum CullModeFlags : unsigned char
{
    None = 0x00,
    Front = 0x01,
    Back = 0x02,
    FrontAndBack = Front | Back,
};

enum PolygonWinding : int
{
    CounterClockwise = 0,
    Clockwise
};