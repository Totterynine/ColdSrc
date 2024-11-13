#pragma once
#include "rendersystem/irendersystem.h"
#include "vulkan_common.h"

class RenderTargetVk : public IRenderTarget
{
public:

	virtual void Create(BufferFormat fmt, int width, int height);
	virtual void Destroy();
	virtual HImage GetHardwareImage();
	virtual HImageView GetHardwareImageView();

	VkImage& GetImage();
	VkImageView& GetImageView();
	void GetExtent(int &width, int &height);

private:
	VkImage renderImage;
	VkImageView imageView;
	VkFormat renderFormat;
	VkExtent2D imageExtent;
	VmaAllocation allocation;
};