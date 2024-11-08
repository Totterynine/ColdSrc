#pragma once
#include "rendersystem/irendersystem.h"
#include "vulkan_common.h"
#include "utils.h"

class DescriptorSetVk : public IDescriptorSet
{
public:
	virtual void SetShaderStages(ShaderStage stages);
	virtual void AddBinding(uint32_t binding, DescriptorType type);
	virtual void BuildLayout();

	virtual void BindImage(uint32_t binding, HImageView img);

	virtual void BuildSet();

	VkDescriptorSetLayout &GetLayout()
	{
		return Layout;
	}

	VkDescriptorSet& GetDescriptor()
	{
		return DescriptorSet;
	}

private:

	VkShaderStageFlagBits AllowedStages = {};
	RenderUtils::DescriptorLayoutBuilder LayoutBuilder;
	VkDescriptorSetLayout Layout;

	VkDescriptorSet DescriptorSet;

	struct BindImageInfo
	{
		uint32_t binding;
		VkDescriptorImageInfo dscImgInfo;
	};
	Array<BindImageInfo> ImageBindings;

	Array<VkWriteDescriptorSet> DescriptorBindings;
};

class ShaderVk : public IShader
{
public:

	virtual ShaderType GetType();
	virtual void SetComputeModule(HShader csModule);

	virtual void BuildPipeline(IDescriptorSet *set);

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