#pragma once
#include "common_stl.h"
#include "rendersystem/irendersystem.h"

#include "VkBootstrap.h"
#include "vk_mem_alloc.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#include "vulkan/vulkan_win32.h"
#endif

class RenderSystemVulkan : public IRenderSystem
{
public:
	virtual void *GetInterface() { return static_cast<IRenderSystem *>(this); }

public:
	virtual bool Create();

	virtual void AttachWindow(void *window_handle, int w, int h);

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
	virtual void SetShader(IShader *shader);

	// Set the vertex buffer
	virtual void SetVertexBuffer(IVertexBuffer *buffer);

	// Set the index buffer
	virtual void SetIndexBuffer(IIndexBuffer *buffer);

	// Draw a primitive
	virtual void DrawPrimitive(int primitive_type, int vertex_count);

	// Draw indexed primitives
	virtual void DrawIndexedPrimitives(int primitive_type, int index_count);

	// Present the render target to surface
	virtual void Present();

	// Destroy the rendering system
	virtual void Destroy();

	// Set the blend state
	virtual void SetBlendState(BlendState settings);

	// Set the depth stencil state
	virtual void SetDepthStencilState(DepthStencilState settings);

	// Set the rasterizer state
	virtual void SetRasterizerState(RasterizerState settings);

private:
	bool CreateQueues();
	bool CreateSwapchain(int w, int h);
	bool RecreateSwapchain();

	bool CreateRenderPass();
	bool CreateFramebuffers();

	bool CreateCommandPool();
	bool CreateCommandBuffers();

	bool CreateSyncObjects();

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

	uint32_t CurrentFrameIdx = 0;
	uint32_t CurrentImageIdx = 0;

	// This is the main backbuffer of the window surface.
	// When no render target is assigned, this is what is used.
	struct BackbufferInfo
	{
		Array<VkImage> Images;
		Array<VkImageView> ImageViews;
		Array<VkFramebuffer> Framebuffers;
		VkRenderPass RenderPass;
	} Backbuffer;

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

	Array<VkSemaphore> AvailableSemaphore;
	Array<VkSemaphore> FinishedSemaphore;
	Array<VkFence> FencesInFlight;
	Array<VkFence> ImagesInFlight;
};