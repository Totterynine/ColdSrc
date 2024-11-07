#pragma once
#include "rendersystem.h"

namespace RenderUtils
{
	VkImageCreateInfo image_create_info(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent, VkImageType type = VK_IMAGE_TYPE_2D, uint32_t mipLevels = 1, uint32_t arrayLayers = 1);
	VkImageViewCreateInfo imageview_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags, VkImageViewType type = VK_IMAGE_VIEW_TYPE_2D, uint32_t mipLevels = 1, uint32_t arrayLayers = 1);
	VkSemaphoreSubmitInfo semaphore_submit_info(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);

	VkFormat ImageFormatToVulkan(ImageFormat fmt);
}