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

VkPipelineShaderStageCreateInfo RenderUtils::shader_stage_create_info(VkShaderStageFlagBits stage, VkShaderModule shader)
{
    VkPipelineShaderStageCreateInfo stageinfo{};
    stageinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageinfo.pNext = nullptr;
    stageinfo.stage = stage;
    stageinfo.module = shader;
    stageinfo.pName = "main";

    return stageinfo;
}

VkRenderingInfo RenderUtils::rendering_info(VkExtent2D renderExtent, VkRenderingAttachmentInfo* colorAttachment, VkRenderingAttachmentInfo* depthAttachment, uint32_t attachment_count)
{
    VkRenderingInfo renderInfo{};
    renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderInfo.pNext = nullptr;

    renderInfo.renderArea = VkRect2D{ VkOffset2D { 0, 0 }, renderExtent };
    renderInfo.layerCount = 1;
    renderInfo.colorAttachmentCount = attachment_count;
    renderInfo.pColorAttachments = colorAttachment;
    renderInfo.pDepthAttachment = depthAttachment;
    renderInfo.pStencilAttachment = nullptr;

    return renderInfo;
}

VkRenderingAttachmentInfo RenderUtils::attachment_info(VkImageView view, VkClearValue* clear, VkImageLayout layout)
{
    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.pNext = nullptr;

    colorAttachment.imageView = view;
    colorAttachment.imageLayout = layout;
    colorAttachment.loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    if (clear) {
        colorAttachment.clearValue = *clear;
    }

    return colorAttachment;
}

VkFormat RenderUtils::BufferFormatToVulkan(BufferFormat fmt)
{
    switch (fmt)
    {
    case BufferFormat::Null:
        return VK_FORMAT_A8_UNORM_KHR; // smallest i could find, does vulkan even have NULL format?
    case BufferFormat::R8:
        return VK_FORMAT_R8_UINT;
    case BufferFormat::R16:
        return VK_FORMAT_R16_UINT;
    case BufferFormat::R32:
        return VK_FORMAT_R32_UINT;
    case BufferFormat::R16F:
        return VK_FORMAT_R16_SFLOAT;
    case BufferFormat::R32F:
        return VK_FORMAT_R32_SFLOAT;
    case BufferFormat::RG8:
        return VK_FORMAT_R8G8_UINT;
    case BufferFormat::RG16:
        return VK_FORMAT_R16G16_UINT;
    case BufferFormat::RG32:
        return VK_FORMAT_R32G32_UINT;
    case BufferFormat::RG16F:
        return VK_FORMAT_R16G16_SFLOAT;
    case BufferFormat::RG32F:
        return VK_FORMAT_R32G32_SFLOAT;
    case BufferFormat::RGB8:
        return VK_FORMAT_R8G8B8_UINT;
    case BufferFormat::RGB16:
        return VK_FORMAT_R16G16B16_UINT;
    case BufferFormat::RGB32:
        return VK_FORMAT_R32G32B32_UINT;
    case BufferFormat::RGB16F:
        return VK_FORMAT_R16G16B16_SFLOAT;
    case BufferFormat::RGB32F:
        return VK_FORMAT_R32G32B32_SFLOAT;
    case BufferFormat::RGBA8:
        return VK_FORMAT_R8G8B8A8_UINT;
    case BufferFormat::RGBA16:
        return VK_FORMAT_R16G16B16A16_UINT;
    case BufferFormat::RGBA32:
        return VK_FORMAT_R32G32B32A32_UINT;
    case BufferFormat::RGBA16F:
        return VK_FORMAT_R16G16B16A16_SFLOAT;
    case BufferFormat::RGBA32F:
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
    case DescriptorType::ConstantBuffer :
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case DescriptorType::StorageBuffer:
        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case DescriptorType::StorageImage:
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

VkPrimitiveTopology RenderUtils::PrimitiveTopologyToVulkan(PrimitiveTopology topology)
{
    switch (topology)
    {
    case Points:
        return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case Lines:
        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case LineStrip:
        return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case Triangles:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    default:
        return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
    }
}

VkPolygonMode RenderUtils::PolygonModeToVulkan(PolygonMode mode)
{
    switch (mode)
    {
    case Fill:
        return VK_POLYGON_MODE_FILL;
    case Line:
        return VK_POLYGON_MODE_LINE;
    case Point:
        return VK_POLYGON_MODE_POINT;
    default:
        return VK_POLYGON_MODE_MAX_ENUM;
    }
}

VkCullModeFlagBits RenderUtils::CullModeFlagsToVulkan(CullModeFlags flags)
{
    VkCullModeFlagBits flag_bits = VK_CULL_MODE_NONE;

    if (flags & CullModeFlags::Front)
        flag_bits = static_cast<VkCullModeFlagBits>(flag_bits | VK_CULL_MODE_FRONT_BIT);
    if (flags & CullModeFlags::Back)
        flag_bits = static_cast<VkCullModeFlagBits>(flag_bits | VK_CULL_MODE_BACK_BIT);

    return flag_bits;
}

VkFrontFace RenderUtils::PolygonWindingToVulkan(PolygonWinding winding)
{
    switch (winding)
    {
    case CounterClockwise:
        return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    case Clockwise:
        return VK_FRONT_FACE_CLOCKWISE;
    default:
        return VK_FRONT_FACE_MAX_ENUM;
    }
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

void RenderUtils::GraphicsPipelineBuilder::SetShaders(VkShaderModule vertexShader, VkShaderModule fragmentShader)
{
    Stages.clear();

    Stages.push_back(RenderUtils::shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, vertexShader));
    Stages.push_back(RenderUtils::shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader));
}

void RenderUtils::GraphicsPipelineBuilder::SetTopology(VkPrimitiveTopology topology)
{
    InputAssembly.topology = topology;
    InputAssembly.primitiveRestartEnable = VK_FALSE;
}

void RenderUtils::GraphicsPipelineBuilder::SetPolygonMode(VkPolygonMode polygonMode)
{
    Rasterizer.polygonMode = polygonMode;
}

void RenderUtils::GraphicsPipelineBuilder::SetCullMode(VkCullModeFlagBits cullFlags, VkFrontFace frontFace)
{
    Rasterizer.cullMode = cullFlags;
    Rasterizer.frontFace = frontFace;
}

void RenderUtils::GraphicsPipelineBuilder::Clear()
{
    InputAssembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    Rasterizer = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    ColorBlendAttachment = {};
    Multisampling = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    PipelineLayout = {};
    DepthStencil = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
    RenderInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };

    // Default to no MSAA
    Multisampling.sampleShadingEnable = VK_FALSE;
    Multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    Multisampling.minSampleShading = 1.0f;
    Multisampling.pSampleMask = nullptr;
    Multisampling.alphaToCoverageEnable = VK_FALSE;
    Multisampling.alphaToOneEnable = VK_FALSE;

    // default write mask, disable blending (for now)
    ColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    ColorBlendAttachment.blendEnable = VK_FALSE;

    Rasterizer.lineWidth = 1.0f;

    Stages.clear();
}

VkPipeline RenderUtils::GraphicsPipelineBuilder::Build(VkDevice device)
{
    // make viewport state from our stored viewport and scissor.
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = nullptr;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // dummy color blending. We arent using transparent objects (yet)
    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.pNext = nullptr;

    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &ColorBlendAttachment;

    // dummy VertexInputStateCreateInfo, we have no need for it
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

    // build the pipeline create structure
    VkGraphicsPipelineCreateInfo pipelineInfo = { .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    pipelineInfo.pNext = &RenderInfo;

    pipelineInfo.stageCount = (uint32_t)Stages.size();
    pipelineInfo.pStages = Stages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &InputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &Rasterizer;
    pipelineInfo.pMultisampleState = &Multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDepthStencilState = &DepthStencil;
    pipelineInfo.layout = PipelineLayout;

    VkDynamicState state[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
    dynamicInfo.pDynamicStates = &state[0];
    dynamicInfo.dynamicStateCount = 2;

    pipelineInfo.pDynamicState = &dynamicInfo;

    VkPipeline newPipeline;
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS) 
    {
        return VK_NULL_HANDLE; // failed to create graphics pipeline
    }
    else 
    {
        return newPipeline;
    }
}
