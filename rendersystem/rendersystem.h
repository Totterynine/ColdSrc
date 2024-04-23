#pragma once
#include "common_stl.h"
#include "rendersystem/irendersystem.h"
#include "libcommon/module_lib.h"

#include "VkBootstrap.h"
#include "vk_mem_alloc.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#include "vulkan/vulkan_win32.h"
#endif

class RenderSystemVulkan : public IRenderSystem, public IModule
{
public:

	RenderSystemVulkan() : IModule("RenderSystem") {}
	virtual void *GetInterface() { return static_cast<IRenderSystem *>( this ); }

public:

	virtual bool CreateRenderer();

	virtual void AttachWindow( void *window_handle, int w, int h );

private:

	bool CreateQueues();
	bool CreateSwapchain(int w, int h);
	bool CreateRenderPass();

	vkb::Instance VulkanInstance;

	struct RenderWindow
	{
		void *Handle = nullptr;
		vkb::Swapchain SwapChain = {};
		VkSurfaceKHR vkSurface;
	} CurrentWindow;

	struct DeviceStore
	{
		vkb::PhysicalDevice Physical;
		vkb::Device Logical;
		vkb::DispatchTable Dispatch;
	} Device;

	VkQueue GraphicsQueue;
	VkQueue PresentQueue;

	VkRenderPass RenderPass;
};