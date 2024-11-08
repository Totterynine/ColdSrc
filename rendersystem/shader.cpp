#include "common_stl.h"
#include "shader.h"
#include "rendersystem.h"
#include "descriptorsets.h"

ShaderType ShaderVk::GetType()
{
	return Type;
}

void ShaderVk::SetComputeModule(HShader csModule)
{
	Type = ShaderType::Compute;

	ComputeShader = static_cast<VkShaderModule>(csModule);
}

void ShaderVk::BuildPipeline(IDescriptorLayout* layout)
{
	descriptorLayout = static_cast<DescriptorLayoutVk*>(layout)->GetLayout();

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