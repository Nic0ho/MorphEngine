#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

#include "Common/MorphTypes.h"

namespace MorphVK
{
void BeginCommandBuffer(VkCommandBuffer CommandBuffer, VkCommandBufferUsageFlags usageFlags);

VkSemaphore CreateSemaphore(VkDevice Device);
}