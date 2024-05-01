#include "rendersystem.h"

static Modules::DeclareModule<RenderSystemVulkan> rendersystem;

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

bool RenderSystemVulkan::Create()
{
    vkb::InstanceBuilder builder;
    auto inst_ret = builder.set_app_name("ColdSrc")
                        .request_validation_layers()
                        .use_default_debug_messenger()
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

    vkb::PhysicalDeviceSelector selector{VulkanInstance};
    auto phys_ret = selector.set_surface(CurrentWindow.vkSurface)
                        .set_minimum_version(1, 2)
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
    if (!CreateRenderPass())
        return;
    if (!CreateFramebuffers())
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
    Device.Dispatch.waitForFences(1, &FencesInFlight[CurrentFrameIdx], VK_TRUE, UINT64_MAX);

    Device.Dispatch.resetCommandBuffer(CommandBuffers[CurrentFrameIdx], 0);

    VkResult result = Device.Dispatch.acquireNextImageKHR(CurrentWindow.SwapChain, UINT64_MAX, AvailableSemaphore[CurrentFrameIdx], NULL, &CurrentImageIdx);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapchain();
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        // std::cout << "failed to acquire swapchain image. Error " << result << "\n";
        return;
    }

    if (ImagesInFlight[CurrentImageIdx] != VK_NULL_HANDLE)
    {
        Device.Dispatch.waitForFences(1, &ImagesInFlight[CurrentImageIdx], VK_TRUE, UINT64_MAX);
    }
    ImagesInFlight[CurrentImageIdx] = FencesInFlight[CurrentFrameIdx];

    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (Device.Dispatch.beginCommandBuffer(CommandBuffers[CurrentFrameIdx], &begin_info) != VK_SUCCESS)
    {
        return; // failed to begin recording command buffer
    }

    VkRenderPassBeginInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = RenderPass;
    render_pass_info.framebuffer = Framebuffer.Framebuffers[CurrentImageIdx];
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = {CurrentWindow.Width, CurrentWindow.Height};

    VkClearValue clearColor{{{0.0f, 0.0f, 0.0f, 1.0f}}};
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clearColor;

    Device.Dispatch.cmdBeginRenderPass(CommandBuffers[CurrentFrameIdx], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderSystemVulkan::EndRendering()
{
    Device.Dispatch.cmdEndRenderPass(CommandBuffers[CurrentFrameIdx]);

    if (Device.Dispatch.endCommandBuffer(CommandBuffers[CurrentFrameIdx]) != VK_SUCCESS)
    {
        // std::cout << "failed to record command buffer\n";
        return; // failed to record command buffer!
    }

    if (ImagesInFlight[CurrentImageIdx] != VK_NULL_HANDLE)
    {
        Device.Dispatch.waitForFences(1, &ImagesInFlight[CurrentImageIdx], VK_TRUE, UINT64_MAX);
    }
    ImagesInFlight[CurrentImageIdx] = FencesInFlight[CurrentFrameIdx];

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {AvailableSemaphore[CurrentFrameIdx]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &CommandBuffers[CurrentFrameIdx];

    VkSemaphore signalSemaphores[] = {FinishedSemaphore[CurrentFrameIdx]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    Device.Dispatch.resetFences(1, &FencesInFlight[CurrentFrameIdx]);

    Device.Dispatch.queueSubmit(GraphicsQueue, 1, &submitInfo, FencesInFlight[CurrentFrameIdx]);
}

void RenderSystemVulkan::SetClearColor(ColorFloat &color)
{
    ClearColorValue = {color.r, color.g, color.b, color.a};
    ClearValue.color = ClearColorValue;
}

void RenderSystemVulkan::ClearColor()
{
    VkClearAttachment clearAttachments[] = {
        {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
         .colorAttachment = 0,
         .clearValue = ClearValue}};

    VkClearRect clearRect = {
        .rect = {
            .offset = {CurrentViewport.x, CurrentViewport.y},
            .extent = {static_cast<uint32_t>(CurrentViewport.w), static_cast<uint32_t>(CurrentViewport.h)},
        },
        .baseArrayLayer = 0,
        .layerCount = 1};

    Device.Dispatch.cmdClearAttachments(CommandBuffers[CurrentFrameIdx], 1, clearAttachments, 1, &clearRect);
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
    VkSemaphore signal_semaphores[] = {FinishedSemaphore[CurrentFrameIdx]};

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swapChains[] = {CurrentWindow.SwapChain};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapChains;

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

    for (auto framebuffer : Framebuffer.Framebuffers)
    {
        Device.Dispatch.destroyFramebuffer(framebuffer, nullptr);
    }

    CurrentWindow.SwapChain.destroy_image_views(Framebuffer.ImageViews);

    if (!CreateSwapchain(CurrentWindow.Width, CurrentWindow.Height) &&
        !CreateFramebuffers() &&
        !CreateCommandPool() &&
        !CreateCommandBuffers())
        return false;

    return true;
}

bool RenderSystemVulkan::CreateRenderPass()
{
    VkAttachmentDescription color_attachment = {};
    color_attachment.format = CurrentWindow.SwapChain.image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref = {};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &color_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    if (Device.Dispatch.createRenderPass(&render_pass_info, nullptr, &RenderPass) != VK_SUCCESS)
    {
        // std::cout << "failed to create render pass\n";
        return false; // failed to create render pass!
    }
    return true;
}

bool RenderSystemVulkan::CreateFramebuffers()
{
    Framebuffer.Images = CurrentWindow.SwapChain.get_images().value();
    Framebuffer.ImageViews = CurrentWindow.SwapChain.get_image_views().value();

    Framebuffer.Framebuffers.resize(Framebuffer.ImageViews.size());

    for (size_t i = 0; i < Framebuffer.ImageViews.size(); i++)
    {
        VkImageView attachments[] = {Framebuffer.ImageViews[i]};

        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = RenderPass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = attachments;
        framebuffer_info.width = CurrentWindow.SwapChain.extent.width;
        framebuffer_info.height = CurrentWindow.SwapChain.extent.height;
        framebuffer_info.layers = 1;

        if (Device.Dispatch.createFramebuffer(&framebuffer_info, nullptr, &Framebuffer.Framebuffers[i]) != VK_SUCCESS)
        {
            return false; // failed to create framebuffer
        }
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
    CommandBuffers.resize(Framebuffer.Framebuffers.size());

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
    AvailableSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
    FinishedSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
    FencesInFlight.resize(MAX_FRAMES_IN_FLIGHT);
    ImagesInFlight.resize(CurrentWindow.SwapChain.image_count, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (Device.Dispatch.createSemaphore(&semaphore_info, nullptr, &AvailableSemaphore[i]) != VK_SUCCESS ||
            Device.Dispatch.createSemaphore(&semaphore_info, nullptr, &FinishedSemaphore[i]) != VK_SUCCESS ||
            Device.Dispatch.createFence(&fence_info, nullptr, &FencesInFlight[i]) != VK_SUCCESS)
        {
            // std::cout << "failed to create sync objects\n";
            return false; // failed to create synchronization objects for a frame
        }
    }
    return true;
}
