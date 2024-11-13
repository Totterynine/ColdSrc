#include "common_stl.h"
#include "shader.h"
#include "rendersystem.h"
#include "descriptorsets.h"

ShaderType ShaderVk::GetType()
{
	return Type;
}

void ShaderVk::SetVertexModule(HShader vsModule)
{
	Type = ShaderType::Graphics;
	VertexShader = static_cast<VkShaderModule>(vsModule);
}

void ShaderVk::SetFragmentModule(HShader fsModule)
{
	Type = ShaderType::Graphics;
	FragmentShader = static_cast<VkShaderModule>(fsModule);
}

void ShaderVk::SetComputeModule(HShader csModule)
{
	Type = ShaderType::Compute;
	ComputeShader = static_cast<VkShaderModule>(csModule);
}

void ShaderVk::SetTopology(PrimitiveTopology topology)
{
	PipelineBuilder.SetTopology(RenderUtils::PrimitiveTopologyToVulkan(topology));
}

void ShaderVk::SetPolygonMode(PolygonMode polygonMode)
{
	PipelineBuilder.SetPolygonMode(RenderUtils::PolygonModeToVulkan(polygonMode));
}

void ShaderVk::SetCullMode(CullModeFlags cullFlags, PolygonWinding winding)
{
	PipelineBuilder.SetCullMode(RenderUtils::CullModeFlagsToVulkan(cullFlags), RenderUtils::PolygonWindingToVulkan(winding));
}

void ShaderVk::BuildPipeline(IDescriptorLayout* layout)
{
	if(layout != nullptr)
		descriptorLayout = static_cast<DescriptorLayoutVk*>(layout)->GetLayout();

	switch (Type)
	{
	case ShaderType::Null:
		break;
	case ShaderType::Graphics:
		BuildGraphicsPipeline();
		break;
	case ShaderType::Compute:
		BuildComputePipeline();
		break;
	default:
		break;
	}
}

void ShaderVk::Destroy()
{
	vkDestroyPipelineLayout(rendersystem->GetDevice(), shaderPipelineLayout, nullptr);
	vkDestroyPipeline(rendersystem->GetDevice(), shaderPipeline, nullptr);

	if (FragmentShader != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule(rendersystem->GetDevice(), FragmentShader, nullptr);
	}
	if (VertexShader != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule(rendersystem->GetDevice(), VertexShader, nullptr);
	}
	if (ComputeShader != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule(rendersystem->GetDevice(), ComputeShader, nullptr);
	}
}

void ShaderVk::BuildGraphicsPipeline()
{
	PipelineBuilder.SetShaders(VertexShader, FragmentShader);

	VkPipelineLayoutCreateInfo graphicsLayout{};
	graphicsLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	if (descriptorLayout != VK_NULL_HANDLE)
	{
		graphicsLayout.pNext = nullptr;
		graphicsLayout.pSetLayouts = &descriptorLayout;
		graphicsLayout.setLayoutCount = 1;
	}

	vkCreatePipelineLayout(rendersystem->GetDevice(), &graphicsLayout, nullptr, &shaderPipelineLayout);

	PipelineBuilder.PipelineLayout = shaderPipelineLayout;

	shaderPipeline = PipelineBuilder.Build(rendersystem->GetDevice());
}

void ShaderVk::BuildComputePipeline()
{
	VkPipelineLayoutCreateInfo computeLayout{};
	computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	computeLayout.pNext = nullptr;
	computeLayout.pSetLayouts = &descriptorLayout;
	computeLayout.setLayoutCount = 1;

	vkCreatePipelineLayout(rendersystem->GetDevice(), &computeLayout, nullptr, &shaderPipelineLayout);

	VkPipelineShaderStageCreateInfo stageinfo = RenderUtils::shader_stage_create_info(VK_SHADER_STAGE_COMPUTE_BIT, ComputeShader);

	VkComputePipelineCreateInfo computePipelineCreateInfo{};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.pNext = nullptr;
	computePipelineCreateInfo.layout = shaderPipelineLayout;
	computePipelineCreateInfo.stage = stageinfo;

	vkCreateComputePipelines(rendersystem->GetDevice(), VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &shaderPipeline);
}