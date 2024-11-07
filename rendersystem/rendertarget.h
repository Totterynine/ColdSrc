#pragma once
#include "rendersystem.h"

class RenderTargetVk : public IRenderTarget
{
public:

	virtual void Create(ImageFormat fmt, int width, int height);
	virtual void Destroy();

	VkImage& GetImage();
	void GetExtent(int &width, int &height);

private:
	VkImage renderImage;
	VkImageView imageView;
	VkFormat renderFormat;
	VkExtent2D imageExtent;
	VmaAllocation allocation;
};