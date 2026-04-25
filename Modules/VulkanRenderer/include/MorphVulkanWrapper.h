#pragma once
#include <vulkan/vulkan.h>

#include "Common/MorphTypes.h"

namespace MorphVK
{
 
void BeginCommandBuffer(VkCommandBuffer CommandBuffer, VkCommandBufferUsageFlags usageFlags);
 
VkSemaphore CreateSemaphore(VkDevice Device);

VkImageView CreateImageView(VkDevice Device, VkImage Image, VkFormat Format, VkImageAspectFlags AspectFlags);
 
VkSampler CreateTextureSampler(VkDevice Device, VkFilter MinFilter, VkFilter MaxFilter, VkSamplerAddressMode AddressMode);
 
void ImageMemBarrier(VkCommandBuffer CmdBuf, VkImage Image, VkFormat Format, VkImageLayout OldLayout, VkImageLayout NewLayout);
 
}