#pragma once
#include "rendersystem/irendersystem.h"
#include "vulkan_common.h"
#include "utils.h"

class ShaderVk : public IShader
{
public:

	virtual ShaderType GetType();
	virtual void SetComputeModule(HShader csModule);

	virtual void BuildPipeline(IDescriptorLayout *layout);

	VkPipeline GetPipeline()
	{
		return shaderPipeline;
	}

	VkPipelineLayout GetPipelineLayout()
	{
		return shaderPipelineLayout;
	}

private:

	void BuildComputePipeline();

	VkPipeline shaderPipeline;
	VkPipelineLayout shaderPipelineLayout;
	ShaderType Type;

	VkDescriptorSetLayout descriptorLayout;

	VkShaderModule FragmentShader;
	VkShaderModule VertexShader;
	VkShaderModule ComputeShader;
};