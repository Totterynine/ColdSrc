#include "common_stl.h"
#include "rendertarget.h"
#include "utils.h"
#include "rendersystem.h"

void RenderTargetVk::Create(BufferFormat fmt, int width, int height)
{
	renderFormat = RenderUtils::BufferFormatToVulkan(fmt);
	imageExtent.width = width;
	imageExtent.height = height;

	VkImageUsageFlags drawImageUsages{};
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	VkImageCreateInfo imgInfo = RenderUtils::image_create_info(renderFormat, drawImageUsages, { imageExtent.width, imageExtent.height, 1 });

	//for the render target, we want to allocate it from gpu local memory
	VmaAllocationCreateInfo rimg_allocinfo = {};
	rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	//allocate and create the image
	vmaCreateImage(rendersystem->GetAllocator(), &imgInfo, &rimg_allocinfo, &renderImage, &allocation, nullptr);

	//build a image-view for the draw image to use for rendering
	VkImageViewCreateInfo rview_info = RenderUtils::imageview_create_info(renderFormat, renderImage, VK_IMAGE_ASPECT_COLOR_BIT);

	vkCreateImageView(rendersystem->GetDevice(), &rview_info, nullptr, &imageView);
}

void RenderTargetVk::Destroy()
{
	vkDestroyImageView(rendersystem->GetDevice(), imageView, nullptr);
	vmaDestroyImage(rendersystem->GetAllocator(), renderImage, allocation);
}

HImage RenderTargetVk::GetHardwareImage()
{
	return GetImage();
}

HImageView RenderTargetVk::GetHardwareImageView()
{
	return GetImageView();
}

VkImage& RenderTargetVk::GetImage()
{
	return renderImage;
}

VkImageView& RenderTargetVk::GetImageView()
{
	return imageView;
}

void RenderTargetVk::GetExtent(int& width, int& height)
{
	width = imageExtent.width;
	height = imageExtent.height;
}
