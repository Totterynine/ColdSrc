#include "rendersystem.h"

static Modules::DeclareModule<RenderSystemVulkan> rendersystem;

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

bool RenderSystemVulkan::Create()
{
    vkb::InstanceBuilder builder;
    auto inst_ret = builder.set_app_name("ColdSrc")
                        .request_validation_layers(true)
                        .use_default_debug_messenger()
                        .require_api_version(1, 3, 0)
                        .build();
    if (!inst_ret)
    {
        // std::cerr << "Failed to create Vulkan instance. Error: " << inst_ret.error().message() << "\n";
        return false;
    }

    VulkanInstance = inst_ret.value();

    return true;
}

void RenderSystemVulkan::AttachWindow(void *window_handle, int width, int height)
{
    CurrentWindow.Handle = window_handle;

    CurrentWindow.Width = width;
    CurrentWindow.Height = height;

#ifdef WIN32
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = *(HWND *)window_handle;
    createInfo.hinstance = GetModuleHandle(nullptr);

    if (vkCreateWin32SurfaceKHR(VulkanInstance, &createInfo, nullptr, &CurrentWindow.vkSurface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
#endif

    //vulkan 1.3 features
    VkPhysicalDeviceVulkan13Features features13{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
    features13.dynamicRendering = true;
    features13.synchronization2 = true;

    //vulkan 1.2 features
    VkPhysicalDeviceVulkan12Features features12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
    features12.bufferDeviceAddress = true;
    features12.descriptorIndexing = true;

    vkb::PhysicalDeviceSelector selector{VulkanInstance};
    auto phys_ret = selector.set_surface(CurrentWindow.vkSurface)
                        .set_minimum_version(1, 3)
                        .set_required_features_13(features13)
                        .set_required_features_12(features12)
                        .require_dedicated_transfer_queue()
                        .select();
    if (!phys_ret)
    {
        // std::cerr << "Failed to select Vulkan Physical Device. Error: " << phys_ret.error().message() << "\n";
        return;
    }

    Device.Physical = phys_ret.value();

    vkb::DeviceBuilder device_builder{Device.Physical};
    // automatically propagate needed data from instance & physical device
    auto dev_ret = device_builder.build();
    if (!dev_ret)
    {
        // std::cerr << "Failed to create Vulkan device. Error: " << dev_ret.error().message() << "\n";
        return;
    }

    Device.Logical = dev_ret.value();

    Device.Dispatch = Device.Logical.make_table();

    if (!CreateQueues())
        return;
    if (!CreateSwapchain(width, height))
        return;
    if (!CreateBackBufferObjects())
        return;
    if (!CreateCommandPool())
        return;
    if (!CreateCommandBuffers())
        return;
    if (!CreateSyncObjects())
        return;
}

void RenderSystemVulkan::BeginRendering()
{
    // wait for GPU to finish the work, then reset the fence
    Device.Dispatch.waitForFences(1, &SwapChainSyncObjects[CurrentFrameIdx].Fence, VK_TRUE, UINT64_MAX);
    Device.Dispatch.resetFences(1, &SwapChainSyncObjects[CurrentFrameIdx].Fence);

    // request the swapchain image
    VkResult result = Device.Dispatch.acquireNextImageKHR(CurrentWindow.SwapChain, UINT64_MAX, SwapChainSyncObjects[CurrentFrameIdx].SwapSemaphore, NULL, &CurrentImageIdx);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapchain();
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        // std::cout << "failed to acquire swapchain image. Error " << result << "\n";
        return;
    }

    VkCommandBuffer& CommandBuffer = CommandBuffers[CurrentFrameIdx];

    // reset command buffer to begin recording a new one
    Device.Dispatch.resetCommandBuffer(CommandBuffer, 0);

    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // (optional) we use this buffer exactly once in every frame, then reset
    begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (Device.Dispatch.beginCommandBuffer(CommandBuffer, &begin_info) != VK_SUCCESS)
    {
        return; // failed to begin recording command buffer
    }

    // Transition the current image layout as general, so we can render into it
    Util_TransitionImage(CommandBuffer, BackBuffers[CurrentImageIdx].Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
}

void RenderSystemVulkan::EndRendering()
{
    // Transition the current image layout to presentable, so it can be presented
    Util_TransitionImage(CommandBuffers[CurrentFrameIdx], BackBuffers[CurrentImageIdx].Image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    if (Device.Dispatch.endCommandBuffer(CommandBuffers[CurrentFrameIdx]) != VK_SUCCESS)
    {
        // std::cout << "failed to record command buffer\n";
        return; // failed to record command buffer!
    }

    VkCommandBufferSubmitInfo commandSubmitInfo{};
    commandSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    commandSubmitInfo.pNext = nullptr;
    commandSubmitInfo.commandBuffer = CommandBuffers[CurrentFrameIdx];
    commandSubmitInfo.deviceMask = 0;

    VkSemaphoreSubmitInfo waitSemaphoreInfo = Util_CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, SwapChainSyncObjects[CurrentFrameIdx].SwapSemaphore);
    VkSemaphoreSubmitInfo signalSemaphoreInfo = Util_CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, SwapChainSyncObjects[CurrentFrameIdx].RenderSemaphore);

    VkSubmitInfo2 submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submitInfo.pNext = nullptr;

    submitInfo.waitSemaphoreInfoCount = 1;
    submitInfo.pWaitSemaphoreInfos = &waitSemaphoreInfo;

    submitInfo.signalSemaphoreInfoCount = 1;
    submitInfo.pSignalSemaphoreInfos = &signalSemaphoreInfo;

    submitInfo.commandBufferInfoCount = 1;
    submitInfo.pCommandBufferInfos = &commandSubmitInfo;

    Device.Dispatch.queueSubmit2(GraphicsQueue, 1, &submitInfo, SwapChainSyncObjects[CurrentFrameIdx].Fence);
}

void RenderSystemVulkan::SetClearColor(ColorFloat &color)
{
    ClearColorValue = {color.r, color.g, color.b, color.a};
    ClearValue.color = ClearColorValue;
}

void RenderSystemVulkan::ClearColor()
{
    VkImageSubresourceRange imgClearColorRange = {};
    imgClearColorRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imgClearColorRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
    imgClearColorRange.levelCount = VK_REMAINING_MIP_LEVELS;

    Device.Dispatch.cmdClearColorImage(CommandBuffers[CurrentFrameIdx], BackBuffers[CurrentImageIdx].Image, VK_IMAGE_LAYOUT_GENERAL, &ClearColorValue, 1, &imgClearColorRange);
}

void RenderSystemVulkan::SetRenderTarget(IRenderTarget *target)
{
}

void RenderSystemVulkan::SetViewport(Viewport settings)
{
    CurrentViewport = settings;

    VkViewport viewport = {};
    viewport.x = CurrentViewport.x;
    viewport.y = CurrentViewport.y;
    viewport.width = (float)CurrentViewport.w;
    viewport.height = (float)CurrentViewport.h;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    Device.Dispatch.cmdSetViewport(CommandBuffers[CurrentFrameIdx], 0, 1, &viewport);
}

void RenderSystemVulkan::SetScissorRectangle(ScissorRectangle settings)
{
    CurrentScissor = settings;

    VkRect2D scissor = {};
    scissor.offset = {CurrentScissor.x, CurrentScissor.y};
    scissor.extent = {static_cast<uint32_t>(CurrentScissor.w), static_cast<uint32_t>(CurrentScissor.h)};

    Device.Dispatch.cmdSetScissor(CommandBuffers[CurrentFrameIdx], 0, 1, &scissor);
}

void RenderSystemVulkan::SetShader(IShader *shader)
{
}

void RenderSystemVulkan::SetVertexBuffer(IVertexBuffer *buffer)
{
}

void RenderSystemVulkan::SetIndexBuffer(IIndexBuffer *buffer)
{
}

void RenderSystemVulkan::DrawPrimitive(int primitive_type, int vertex_count)
{
}

void RenderSystemVulkan::DrawIndexedPrimitives(int primitive_type, int index_count)
{
}

void RenderSystemVulkan::Present()
{
    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &SwapChainSyncObjects[CurrentFrameIdx].RenderSemaphore;

    present_info.swapchainCount = 1;
    present_info.pSwapchains = &CurrentWindow.SwapChain.swapchain;

    present_info.pImageIndices = &CurrentImageIdx;

    Device.Dispatch.queuePresentKHR(PresentQueue, &present_info);

    CurrentFrameIdx = (CurrentFrameIdx + 1) % MAX_FRAMES_IN_FLIGHT;
}

void RenderSystemVulkan::Destroy()
{
}

void RenderSystemVulkan::SetBlendState(BlendState settings)
{
}

void RenderSystemVulkan::SetDepthStencilState(DepthStencilState settings)
{
}

void RenderSystemVulkan::SetRasterizerState(RasterizerState settings)
{
}

bool RenderSystemVulkan::CreateQueues()
{
    auto graphicsResult = Device.Logical.get_queue(vkb::QueueType::graphics);
    // Device is not suited
    if (!graphicsResult.has_value())
        return false;

    GraphicsQueue = graphicsResult.value();

    auto presentResult = Device.Logical.get_queue(vkb::QueueType::present);
    // Device is not suited
    if (!presentResult.has_value())
        return false;

    PresentQueue = presentResult.value();
    return true;
}

bool RenderSystemVulkan::CreateSwapchain(int w, int h)
{
    vkb::SwapchainBuilder scBuilder(Device.Logical);

    auto swapResult = scBuilder
                          .set_old_swapchain(CurrentWindow.SwapChain)
                          .set_desired_extent(w, h)
                          .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                          .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR) // FIFO is guaranteed to be supported by Vulkan
                          .build();

    if (!swapResult)
        return false;

    vkb::destroy_swapchain(CurrentWindow.SwapChain);

    CurrentWindow.SwapChain = swapResult.value();

    return true;
}

bool RenderSystemVulkan::RecreateSwapchain()
{
    Device.Dispatch.deviceWaitIdle();

    Device.Dispatch.destroyCommandPool(CommandPool, nullptr);

    for (auto backbuffer : BackBuffers)
    {
        Device.Dispatch.destroyFramebuffer(backbuffer.Framebuffer, nullptr);
        vkDestroyImageView(Device.Logical, backbuffer.ImageView, nullptr);
    }

    if (!CreateSwapchain(CurrentWindow.Width, CurrentWindow.Height) &&
        !CreateBackBufferObjects() &&
        !CreateCommandPool() &&
        !CreateCommandBuffers())
        return false;

    return true;
}

bool RenderSystemVulkan::CreateBackBufferObjects()
{
    BackBuffers.resize(CurrentWindow.SwapChain.image_count);

    Array<VkImage> images= CurrentWindow.SwapChain.get_images().value();
    Array<VkImageView> image_views = CurrentWindow.SwapChain.get_image_views().value();

    for (size_t i = 0; i < BackBuffers.size(); i++)
    {
        BackBuffers[i].Image = images[i];
        BackBuffers[i].ImageView = image_views[i];
    }

    return true;
}

bool RenderSystemVulkan::CreateCommandPool()
{
    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = Device.Logical.get_queue_index(vkb::QueueType::graphics).value();

    if (Device.Dispatch.createCommandPool(&pool_info, nullptr, &CommandPool) != VK_SUCCESS)
    {
        // std::cout << "failed to create command pool\n";
        return false; // failed to create command pool
    }

    return true;
}

bool RenderSystemVulkan::CreateCommandBuffers()
{
    CommandBuffers.resize(BackBuffers.size());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = CommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)CommandBuffers.size();

    if (Device.Dispatch.allocateCommandBuffers(&allocInfo, CommandBuffers.data()) != VK_SUCCESS)
    {
        return false; // failed to allocate command buffers;
    }

    return true;
}

bool RenderSystemVulkan::CreateSyncObjects()
{
    SwapChainSyncObjects.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT; // make sure fence is already signaled

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (Device.Dispatch.createSemaphore(&semaphore_info, nullptr, &SwapChainSyncObjects[i].RenderSemaphore) != VK_SUCCESS ||
            Device.Dispatch.createSemaphore(&semaphore_info, nullptr, &SwapChainSyncObjects[i].SwapSemaphore) != VK_SUCCESS ||
            Device.Dispatch.createFence(&fence_info, nullptr, &SwapChainSyncObjects[i].Fence) != VK_SUCCESS)
        {
            // std::cout << "failed to create sync objects\n";
            return false; // failed to create synchronization objects for a frame
        }
    }
    return true;
}

void RenderSystemVulkan::Util_TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout)
{
    VkImageMemoryBarrier2 imageBarrier{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
    imageBarrier.pNext = nullptr;

    imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

    imageBarrier.oldLayout = currentLayout;
    imageBarrier.newLayout = newLayout;

    VkImageAspectFlags aspectMask = (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    VkImageSubresourceRange imgSubresourceRange = {};
    imgSubresourceRange.aspectMask = aspectMask;
    imgSubresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
    imgSubresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;

    imageBarrier.subresourceRange = imgSubresourceRange;
    imageBarrier.image = image;

    VkDependencyInfo depInfo{};
    depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depInfo.pNext = nullptr;

    depInfo.imageMemoryBarrierCount = 1;
    depInfo.pImageMemoryBarriers = &imageBarrier;

    Device.Dispatch.cmdPipelineBarrier2(cmd, &depInfo);
}

VkSemaphoreSubmitInfo RenderSystemVulkan::Util_CreateSemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore)
{
    VkSemaphoreSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.semaphore = semaphore;
    submitInfo.stageMask = stageMask;
    submitInfo.deviceIndex = 0;
    submitInfo.value = 1;

    return submitInfo;
}
