#include "common_stl.h"
#include "utils.h"
#include "rendersystem.h"
#include <fstream>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

void RenderUtils::DescriptorPoolHelper::Init(VkDevice device, uint32_t maxSets, Span<PoolSizeRatio> poolRatios)
{
    Array<VkDescriptorPoolSize> poolSizes;
    for (PoolSizeRatio ratio : poolRatios) {
        poolSizes.push_back(VkDescriptorPoolSize{
            .type = ratio.Type,
            .descriptorCount = uint32_t(ratio.Ratio * maxSets)
            });
    }

    VkDescriptorPoolCreateInfo pool_info = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    pool_info.flags = 0;
    pool_info.maxSets = maxSets;
    pool_info.poolSizeCount = (uint32_t)poolSizes.size();
    pool_info.pPoolSizes = poolSizes.data();

    vkCreateDescriptorPool(device, &pool_info, nullptr, &Pool);
}
void RenderUtils::DescriptorPoolHelper::Clear(VkDevice device)
{
    vkResetDescriptorPool(device, Pool, 0);
}
void RenderUtils::DescriptorPoolHelper::Destroy(VkDevice device)
{
    vkDestroyDescriptorPool(device, Pool, nullptr);
}

VkDescriptorSet RenderUtils::DescriptorPoolHelper::Build(VkDevice device, VkDescriptorSetLayout layout)
{
    VkDescriptorSetAllocateInfo allocInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    allocInfo.pNext = nullptr;
    allocInfo.descriptorPool = Pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    VkDescriptorSet descriptorSet;
    vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);

    return descriptorSet;
}

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

VkShaderModule RenderUtils::load_shader_module(VkDevice device, const char* filePath)
{
    // open the file. With cursor at the end
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        return nullptr;
    }

    // find what the size of the file is by looking up the location of the cursor
    size_t fileSize = (size_t)file.tellg();

    // spirv expects the buffer to be aligned to uint32
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    // put file cursor back to beginning
    file.seekg(0);

    // load the entire file into the buffer
    file.read((char*)buffer.data(), fileSize);

    file.close();

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;

    // codeSize has to be in bytes, multiply by size of int
    createInfo.codeSize = buffer.size() * sizeof(uint32_t);
    createInfo.pCode = buffer.data();

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        return nullptr;
    }

    return shaderModule;
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

VkDescriptorType RenderUtils::DescriptorTypeToVulkan(DescriptorType type)
{
    switch (type)
    {
    case DescriptorType::Buffer:
        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case DescriptorType::Image:
        return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    default:
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }
}

VkPipelineBindPoint RenderUtils::PipelineBindPointToVulkan(PipelineBindPoint type)
{
    switch (type)
    {
    case PipelineBindPoint::Graphics:
        return VK_PIPELINE_BIND_POINT_GRAPHICS;
    case PipelineBindPoint::Compute:
        return VK_PIPELINE_BIND_POINT_COMPUTE;
    default:
        return VK_PIPELINE_BIND_POINT_MAX_ENUM;
    }
}

VkShaderStageFlagBits RenderUtils::ShaderStageToVulkan(ShaderStage stages)
{
    VkShaderStageFlagBits flags = { };

    if (stages & ShaderStage::Vertex)
        flags = static_cast<VkShaderStageFlagBits>(flags | VK_SHADER_STAGE_VERTEX_BIT);
    if (stages & ShaderStage::Pixel)
        flags = static_cast<VkShaderStageFlagBits>(flags | VK_SHADER_STAGE_FRAGMENT_BIT);
    if (stages & ShaderStage::Compute)
        flags = static_cast<VkShaderStageFlagBits>(flags | VK_SHADER_STAGE_COMPUTE_BIT);

    return flags;
}

void RenderUtils::DescriptorLayoutBuilder::AddBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlagBits stage)
{
    VkDescriptorSetLayoutBinding newbind{};
    newbind.binding = binding;
    newbind.descriptorCount = 1;
    newbind.descriptorType = type;
    newbind.stageFlags = stage;

    Bindings.push_back(newbind);
}

void RenderUtils::DescriptorLayoutBuilder::Clear()
{
    Bindings.clear();
}

VkDescriptorSetLayout RenderUtils::DescriptorLayoutBuilder::Build(void *next, VkDescriptorSetLayoutCreateFlags flags)
{
    VkDescriptorSetLayoutCreateInfo info = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    info.pNext = next;

    info.pBindings = Bindings.data();
    info.bindingCount = (uint32_t)Bindings.size();
    info.flags = flags;

    VkDescriptorSetLayout set;
    vkCreateDescriptorSetLayout(rendersystem->GetDevice(), &info, nullptr, &set);

    return set;
}
