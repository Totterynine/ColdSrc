#pragma once
#include "rendersystem/irendersystem.h"
#include "vulkan_common.h"
#include "utils.h"

class DescriptorLayoutVk : public IDescriptorLayout
{
public:
	virtual void AddBinding(uint32_t binding, DescriptorType type, VkShaderStageFlagBits stage);
	virtual void Build();
	
	void Destroy();

	VkDescriptorSetLayout& GetLayout()
	{
		return Layout;
	}

private:

	RenderUtils::DescriptorLayoutBuilder LayoutBuilder;
	VkDescriptorSetLayout Layout;
};

class DescriptorSetVk : public IDescriptorSet
{
public:

	virtual void Init(IDescriptorLayout *layout);

	virtual void BindImage(uint32_t binding, HImageView img);

	virtual void Update();

	VkDescriptorSet& GetDescriptor()
	{
		return DescriptorSet;
	}

private:

	VkDescriptorSet DescriptorSet;

	struct BindImageInfo
	{
		uint32_t binding;
		VkDescriptorImageInfo dscImgInfo;
	};
	Array<BindImageInfo> ImageBindings;

	Array<VkWriteDescriptorSet> DescriptorBindings;
};