#include "common_stl.h"
#include "shader.h"
#include "rendersystem.h"

ShaderType ShaderVk::GetType()
{
	return Type;
}

void ShaderVk::SetComputeModule(HShader csModule)
{
	Type = ShaderType::Compute;

	ComputeShader = static_cast<VkShaderModule>(csModule);
}

void ShaderVk::BuildPipeline(IDescriptorSet* set)
{
	descriptorLayout = static_cast<DescriptorSetVk*>(set)->GetLayout();

	switch (Type)
	{
	case ShaderType::Null:
		break;
	case ShaderType::Graphics:
		break;
	case ShaderType::Compute:
		BuildComputePipeline();
		break;
	default:
		break;
	}
}

void ShaderVk::BuildComputePipeline()
{
	VkPipelineLayoutCreateInfo computeLayout{};
	computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	computeLayout.pNext = nullptr;
	computeLayout.pSetLayouts = &descriptorLayout;
	computeLayout.setLayoutCount = 1;

	vkCreatePipelineLayout(rendersystem->GetDevice(), &computeLayout, nullptr, &shaderPipelineLayout);

	VkPipelineShaderStageCreateInfo stageinfo{};
	stageinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stageinfo.pNext = nullptr;
	stageinfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	stageinfo.module = ComputeShader;
	stageinfo.pName = "main";

	VkComputePipelineCreateInfo computePipelineCreateInfo{};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.pNext = nullptr;
	computePipelineCreateInfo.layout = shaderPipelineLayout;
	computePipelineCreateInfo.stage = stageinfo;

	vkCreateComputePipelines(rendersystem->GetDevice(), VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &shaderPipeline);
}

void DescriptorSetVk::SetShaderStages(ShaderStage stages)
{
	if(stages & ShaderStage::Vertex)
		AllowedStages = static_cast<VkShaderStageFlagBits>(AllowedStages | VK_SHADER_STAGE_VERTEX_BIT);
	if (stages & ShaderStage::Pixel)
		AllowedStages = static_cast<VkShaderStageFlagBits>(AllowedStages | VK_SHADER_STAGE_FRAGMENT_BIT);
	if (stages & ShaderStage::Compute)
		AllowedStages = static_cast<VkShaderStageFlagBits>(AllowedStages | VK_SHADER_STAGE_COMPUTE_BIT);
}

void DescriptorSetVk::AddBinding(uint32_t binding, DescriptorType type)
{
	VkDescriptorType vkType = RenderUtils::DescriptorTypeToVulkan(type);

	LayoutBuilder.AddBinding(0, vkType);
}

void DescriptorSetVk::BuildLayout()
{
	Layout = LayoutBuilder.Build(AllowedStages);

	DescriptorSet = rendersystem->GetDescriptorPool().Build(rendersystem->GetDevice(), Layout);

	LayoutBuilder.Clear();

	ImageBindings.clear();
}

void DescriptorSetVk::BindImage(uint32_t binding, HImageView img)
{
	VkDescriptorImageInfo imgInfo{};
	imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	imgInfo.imageView = static_cast<VkImageView>(img);

	ImageBindings.push_back({ binding, imgInfo });
}

void DescriptorSetVk::BuildSet()
{
	for (auto &imgBind : ImageBindings)
	{
		VkWriteDescriptorSet imageWrite = {};
		imageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		imageWrite.pNext = nullptr;

		imageWrite.dstBinding = imgBind.binding;
		imageWrite.dstSet = DescriptorSet;
		imageWrite.descriptorCount = 1;
		imageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		imageWrite.pImageInfo = &imgBind.dscImgInfo;

		DescriptorBindings.push_back(imageWrite);
	}

	vkUpdateDescriptorSets(rendersystem->GetDevice(), DescriptorBindings.size(), DescriptorBindings.data(), 0, nullptr);
}
