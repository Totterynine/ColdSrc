#pragma once
#include "vulkan_common.h"
#include "rendersystem/irendersystem.h"

namespace RenderUtils
{
	VkImageCreateInfo image_create_info(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent, VkImageType type = VK_IMAGE_TYPE_2D, uint32_t mipLevels = 1, uint32_t arrayLayers = 1);
	VkImageViewCreateInfo imageview_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags, VkImageViewType type = VK_IMAGE_VIEW_TYPE_2D, uint32_t mipLevels = 1, uint32_t arrayLayers = 1);
	VkSemaphoreSubmitInfo semaphore_submit_info(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);
	VkShaderModule load_shader_module(VkDevice device, const char* filePath);
	VkPipelineShaderStageCreateInfo shader_stage_create_info(VkShaderStageFlagBits stage, VkShaderModule shader);
	VkRenderingInfo rendering_info(VkExtent2D renderExtent, VkRenderingAttachmentInfo* colorAttachment, VkRenderingAttachmentInfo* depthAttachment, uint32_t attachment_count = 1);
	VkRenderingAttachmentInfo attachment_info(VkImageView view, VkClearValue* clear = nullptr, VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	VkFormat BufferFormatToVulkan(BufferFormat fmt);
	VkDescriptorType DescriptorTypeToVulkan(DescriptorType type);
	VkPipelineBindPoint PipelineBindPointToVulkan(PipelineBindPoint type);
	VkShaderStageFlagBits ShaderStageToVulkan(ShaderStage stages);

	VkPrimitiveTopology PrimitiveTopologyToVulkan(PrimitiveTopology topology);
	VkPolygonMode PolygonModeToVulkan(PolygonMode mode);
	VkCullModeFlagBits CullModeFlagsToVulkan(CullModeFlags flags);
	VkFrontFace PolygonWindingToVulkan(PolygonWinding winding);

	class GraphicsPipelineBuilder {
	public:
		Array<VkPipelineShaderStageCreateInfo> Stages;

		VkPipelineInputAssemblyStateCreateInfo InputAssembly;
		VkPipelineRasterizationStateCreateInfo Rasterizer;
		VkPipelineColorBlendAttachmentState ColorBlendAttachment;
		VkPipelineMultisampleStateCreateInfo Multisampling;
		VkPipelineLayout PipelineLayout;
		VkPipelineDepthStencilStateCreateInfo DepthStencil;
		VkPipelineRenderingCreateInfo RenderInfo;
		VkFormat ColorAttachmentformat;

		GraphicsPipelineBuilder() { Clear(); }

		void SetShaders(VkShaderModule vertexShader, VkShaderModule fragmentShader);
		void SetTopology(VkPrimitiveTopology topology);
		void SetPolygonMode(VkPolygonMode polygonMode);
		void SetCullMode(VkCullModeFlagBits cullFlags, VkFrontFace frontFace);

		void Clear();

		VkPipeline Build(VkDevice device);
	};

	class DescriptorLayoutBuilder
	{
	public:

		void AddBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlagBits stage);
		void Clear();

		VkDescriptorSetLayout Build(void *next = nullptr, VkDescriptorSetLayoutCreateFlags flags = 0);

		Array<VkDescriptorSetLayoutBinding> Bindings;
	};

	struct DescriptorPoolHelper {

		struct PoolSizeRatio {
			VkDescriptorType Type;
			float Ratio;
		};

		VkDescriptorPool Pool;

		void Init(VkDevice device, uint32_t maxSets, Span<PoolSizeRatio> poolRatios);
		void Clear(VkDevice device);
		void Destroy(VkDevice device);

		VkDescriptorSet Build(VkDevice device, VkDescriptorSetLayout layout);
	};
}