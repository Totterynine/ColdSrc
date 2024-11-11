#pragma once
#include "rendersystem/irendersystem.h"
#include "vulkan_common.h"

class RenderTargetVk : public IRenderTarget
{
public:

	virtual void Create(ImageFormat fmt, int width, int height);
	virtual void Destroy();
	virtual HImage GetHardwareImage();
	virtual HImageView GetHardwareImageView();

	VkImage& GetImage();
	void GetExtent(int &width, int &height);

private:
	VkImage renderImage;
	VkImageView imageView;
	VkFormat renderFormat;
	VkExtent2D imageExtent;
	VmaAllocation allocation;
};