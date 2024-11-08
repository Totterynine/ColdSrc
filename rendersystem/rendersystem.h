#pragma once
#include "common_stl.h"
#include "rendersystem/irendersystem.h"
#include "vulkan_common.h"
#include "utils.h"
#include "shader.h"

#include "vk_mem_alloc.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#include "vulkan/vulkan_win32.h"
#endif

class ReleaseFuncQueue
{
	Queue<std::function<void()>> releaseQueue;
public:

	void Push(std::function<void()>&& func)
	{
		releaseQueue.push_back(func);
	}

	void Release()
	{
		// reverse iterate
		for (auto it = releaseQueue.rbegin(); it != releaseQueue.rend(); it++) {
			(*it)(); //call functors
		}

		releaseQueue.clear();
	}
};

class RenderSystemVulkan : public IRenderSystem
{
public:
	virtual void *GetInterface() { return static_cast<IRenderSystem *>(this); }

public:
	virtual bool Create();

	virtual void AttachWindow(void *window_handle, int w, int h);

	virtual IRenderTarget* CreateRenderTarget(ImageFormat fmt, int width, int height);
	virtual IDescriptorLayout* BuildDescriptorLayout(uint32_t numEntries, DescriptorLayoutEntry* entries);
	virtual IDescriptorSet* BuildDescriptorSet(IDescriptorLayout* layout);
	virtual IShader* CreateShader();
	virtual HShader LoadShaderModule(const char* filepath);

	// Begin rendering
	virtual void BeginRendering();

	// End rendering
	virtual void EndRendering();

	virtual void SetClearColor(ColorFloat &color);
	virtual void ClearColor();

	// Set the render target
	// Set nullptr to clear
	virtual void SetRenderTarget(IRenderTarget *target);

	// Set the viewport
	virtual void SetViewport(Viewport settings);

	// Set the scissor rectangle
	virtual void SetScissorRectangle(ScissorRectangle settings);

	// Set the current shader to render the mesh
	// Set to nullptr to clear
	virtual void BindShader(IShader* shader, PipelineBindPoint point);

	virtual void BindDescriptorSet(IDescriptorSet* set, PipelineBindPoint point);

	// Set the vertex buffer
	virtual void SetVertexBuffer(IVertexBuffer *buffer);

	// Set the index buffer
	virtual void SetIndexBuffer(IIndexBuffer *buffer);

	// Draw a primitive
	virtual void DrawPrimitive(int primitive_type, int vertex_count);

	// Draw indexed primitives
	virtual void DrawIndexedPrimitives(int primitive_type, int index_count);

	virtual void CopyRenderTargetToBackBuffer();

	// Present the render target to surface
	virtual void Present();

	virtual void Dispatch(int groupSizeX, int groupSizeY, int groupSizeZ);

	// Destroy the rendering system
	virtual void Destroy();

	// Set the blend state
	virtual void SetBlendState(BlendState settings);

	// Set the depth stencil state
	virtual void SetDepthStencilState(DepthStencilState settings);

	// Set the rasterizer state
	virtual void SetRasterizerState(RasterizerState settings);

	VmaAllocator &GetAllocator();
	vkb::Device &GetDevice();
	RenderUtils::DescriptorPoolHelper& GetDescriptorPool();

private:
	bool CreateQueues();
	bool CreateSwapchain(int w, int h);
	bool RecreateSwapchain();

	bool CreateBackBufferObjects();

	bool CreateCommandPool();
	bool CreateCommandBuffers();

	bool CreateSyncObjects();

	bool InitDescriptorPool();

	VkImage &GetBoundImage();

	void Cmd_TransitionImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
	void Cmd_BlitImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize);

	vkb::Instance VulkanInstance;

	struct RenderWindow
	{
		void *Handle = nullptr;
		vkb::Swapchain SwapChain = {};
		VkSurfaceKHR vkSurface;

		uint32_t Width, Height;
	} CurrentWindow;

	Viewport CurrentViewport;
	ScissorRectangle CurrentScissor;

	// Current swapchain frame, updated by rendersystem
	uint32_t CurrentFrameIdx = 0;

	// Current swapchain image we are rendering to, retrieved from swapchain
	uint32_t CurrentImageIdx = 0;

	// This is the main backbuffer of the window surface.
	// When no render target is assigned, this is what is used.
	// We have 1 for each swapchain image
	struct BackbufferInfo
	{
		VkImage Image;
		VkImageView ImageView;
		VkFramebuffer Framebuffer;
	};
	Array<BackbufferInfo> BackBuffers;

	struct DeviceStore
	{
		vkb::PhysicalDevice Physical;
		vkb::Device Logical;
		vkb::DispatchTable Dispatch;
	} Device;

	VkQueue GraphicsQueue;
	VkQueue PresentQueue;

	VkCommandPool CommandPool;
	Array<VkCommandBuffer> CommandBuffers; // one for each framebuffer

	VkClearColorValue ClearColorValue;
	VkClearValue ClearValue;

	struct SwapSyncObjects
	{
		VkSemaphore SwapSemaphore;
		VkSemaphore RenderSemaphore;
		VkFence Fence;
	};
	Array<SwapSyncObjects> SwapChainSyncObjects;

	VmaAllocator VulkanAllocator;

	IRenderTarget* BoundRenderTarget = nullptr;
	Array<IRenderTarget*> AllocatedRenderTargets;
	RenderUtils::DescriptorPoolHelper DescriptorPool;

	ShaderVk* BoundShader = nullptr;
};

extern Modules::DeclareModule<RenderSystemVulkan> rendersystem;