#include "common_stl.h"
#include "utils.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

VkImageCreateInfo RenderUtils::image_create_info(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent, VkImageType type, uint32_t mipLevels, uint32_t arrayLayers)
{
    VkImageCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext = nullptr;

    info.imageType = type;

    info.format = format;
    info.extent = extent;

    info.mipLevels = mipLevels;
    info.arrayLayers = arrayLayers;

    //for MSAA. 1 bit indicates there wont be any multisampling
    info.samples = VK_SAMPLE_COUNT_1_BIT;

    //optimal tiling, which means the image is stored on the best gpu format
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = usageFlags;

    return info;
}

VkImageViewCreateInfo RenderUtils::imageview_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags, VkImageViewType type, uint32_t mipLevels, uint32_t arrayLayers)
{
    // build a image-view for the depth image to use for rendering
    VkImageViewCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.pNext = nullptr;

    info.viewType = type;
    info.image = image;
    info.format = format;
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = mipLevels;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = arrayLayers;
    info.subresourceRange.aspectMask = aspectFlags;

    return info;
}

VkSemaphoreSubmitInfo RenderUtils::semaphore_submit_info(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore)
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

VkFormat RenderUtils::ImageFormatToVulkan(ImageFormat fmt)
{
    switch (fmt)
    {
    case ImageFormat::Null:
        return VK_FORMAT_A8_UNORM_KHR; // smallest i could find, does vulkan even have NULL format?
    case ImageFormat::RGB8:
        return VK_FORMAT_R8G8B8_UINT;
    case ImageFormat::RGBA8:
        return VK_FORMAT_R8G8B8A8_UINT;
    case ImageFormat::RGBA16:
        return VK_FORMAT_R16G16B16A16_UINT;
    case ImageFormat::RGBA16F:
        return VK_FORMAT_R16G16B16A16_SFLOAT;
    case ImageFormat::RGBA32F:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    default:
        assert(0);
        return VK_FORMAT_UNDEFINED;
    }
}
