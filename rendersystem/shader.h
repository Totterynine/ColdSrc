#pragma once
#include "rendersystem/irendersystem.h"
#include "rendersystem/ishader.h"
#include "vulkan_common.h"
#include "utils.h"

class ShaderVk : public IShader
{
public:

	virtual ShaderType GetType();

	virtual void SetVertexModule(HShader vsModule);
	virtual void SetFragmentModule(HShader fsModule);
	virtual void SetComputeModule(HShader csModule);

	virtual void SetTopology(PrimitiveTopology topology);
	virtual void SetPolygonMode(PolygonMode polygonMode);
	virtual void SetCullMode(CullModeFlags cullFlags, PolygonWinding winding);

	virtual void BuildPipeline(IDescriptorLayout *layout);

	VkPipeline GetPipeline()
	{
		return shaderPipeline;
	}

	VkPipelineLayout GetPipelineLayout()
	{
		return shaderPipelineLayout;
	}

	void Destroy();

private:

	void BuildGraphicsPipeline();
	void BuildComputePipeline();

	VkPipeline shaderPipeline;
	VkPipelineLayout shaderPipelineLayout;
	ShaderType Type;

	VkDescriptorSetLayout descriptorLayout = VK_NULL_HANDLE;

	VkShaderModule FragmentShader = VK_NULL_HANDLE;
	VkShaderModule VertexShader = VK_NULL_HANDLE;
	VkShaderModule ComputeShader = VK_NULL_HANDLE;
	RenderUtils::GraphicsPipelineBuilder PipelineBuilder;
};