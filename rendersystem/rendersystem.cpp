#include "rendersystem.h"

static Modules::DeclareModule<RenderSystemVulkan> rendersystem;

bool RenderSystemVulkan::CreateRenderer()
{
    vkb::InstanceBuilder builder;
    auto inst_ret = builder.set_app_name( "ColdSrc" )
        .request_validation_layers()
        .use_default_debug_messenger()
        .build();
    if ( !inst_ret ) {
        //std::cerr << "Failed to create Vulkan instance. Error: " << inst_ret.error().message() << "\n";
        return false;
    }

    VulkanInstance = inst_ret.value();

    return true;
}

void RenderSystemVulkan::AttachWindow( void *window_handle, int width, int height )
{
    CurrentWindow.Handle = window_handle;

#ifdef WIN32
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = *(HWND *) window_handle;
    createInfo.hinstance = GetModuleHandle( nullptr );

    if ( vkCreateWin32SurfaceKHR( VulkanInstance, &createInfo, nullptr, &CurrentWindow.vkSurface ) != VK_SUCCESS ) {
        throw std::runtime_error( "failed to create window surface!" );
    }
#endif

    vkb::PhysicalDeviceSelector selector{ VulkanInstance };
    auto phys_ret = selector.set_surface( CurrentWindow.vkSurface )
        .set_minimum_version( 1, 2 )
        .require_dedicated_transfer_queue()
        .select();
    if ( !phys_ret ) {
        //std::cerr << "Failed to select Vulkan Physical Device. Error: " << phys_ret.error().message() << "\n";
        return;
    }

    Device.Physical = phys_ret.value();

    vkb::DeviceBuilder device_builder{ Device.Physical };
    // automatically propagate needed data from instance & physical device
    auto dev_ret = device_builder.build();
    if ( !dev_ret ) {
        //std::cerr << "Failed to create Vulkan device. Error: " << dev_ret.error().message() << "\n";
        return;
    }

    Device.Logical = dev_ret.value();

    Device.Dispatch = Device.Logical.make_table();

    if ( !CreateQueues() )
        return;

    if ( !CreateSwapchain(width, height) )
        return;

    if ( !CreateRenderPass() )
        return;

}

bool RenderSystemVulkan::CreateQueues()
{
    auto graphicsResult = Device.Logical.get_queue( vkb::QueueType::graphics );
    // Device is not suited
    if ( !graphicsResult.has_value() )
        return false;

    GraphicsQueue = graphicsResult.value();

    auto presentResult = Device.Logical.get_queue( vkb::QueueType::present );
    // Device is not suited
    if ( !presentResult.has_value() )
        return false;

    PresentQueue = presentResult.value();
    return true;
}

bool RenderSystemVulkan::CreateSwapchain(int w, int h)
{
    vkb::SwapchainBuilder scBuilder( Device.Logical );

    auto swapResult = scBuilder
        .set_old_swapchain( CurrentWindow.SwapChain )
        .set_desired_extent( w, h )
        .set_desired_present_mode( VK_PRESENT_MODE_FIFO_KHR ) // FIFO is guaranteed to be supported by Vulkan
        .build();

    if ( !swapResult )
        return false;

    vkb::destroy_swapchain( CurrentWindow.SwapChain );

    CurrentWindow.SwapChain = swapResult.value();

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

    if ( Device.Dispatch.createRenderPass( &render_pass_info, nullptr, &RenderPass ) != VK_SUCCESS ) {
        //std::cout << "failed to create render pass\n";
        return false; // failed to create render pass!
    }
    return true;
}
