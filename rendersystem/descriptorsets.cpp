#include "rendersystem.h"
#include "descriptorsets.h"

void DescriptorLayoutVk::AddBinding(uint32_t binding, DescriptorType type, VkShaderStageFlagBits stage)
{
	VkDescriptorType vkType = RenderUtils::DescriptorTypeToVulkan(type);

	LayoutBuilder.AddBinding(0, vkType, stage);
}

void DescriptorLayoutVk::Build()
{
	Layout = LayoutBuilder.Build();

	LayoutBuilder.Clear();
}

void DescriptorLayoutVk::Destroy()
{
	vkDestroyDescriptorSetLayout(rendersystem->GetDevice(), Layout, nullptr);
}

void DescriptorSetVk::Init(IDescriptorLayout *layout)
{
	DescriptorLayoutVk *vkLayout = static_cast<DescriptorLayoutVk*>(layout);
	DescriptorSet = rendersystem->GetDescriptorPool().Build(rendersystem->GetDevice(), vkLayout->GetLayout());

	ImageBindings.clear();
}

void DescriptorSetVk::BindImage(uint32_t binding, HImageView img)
{
	VkDescriptorImageInfo imgInfo{};
	imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	imgInfo.imageView = static_cast<VkImageView>(img);

	ImageBindings.push_back({ binding, imgInfo });
}

void DescriptorSetVk::Update()
{
	for (auto& imgBind : ImageBindings)
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
