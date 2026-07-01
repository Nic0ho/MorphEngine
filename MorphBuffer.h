#pragma once
#include "MorphTypes.h"
#include <vulkan/vulkan.h>

typedef struct
{
    VkBuffer       buffer;
    VkDeviceMemory memory;
} MorphBuffer;

bool morphBufferCreate(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, MorphBuffer* out);
void morphBufferDestroy(VkDevice device, MorphBuffer* buf);
void morphBufferCopy(VkDevice device, VkCommandPool pool, VkQueue queue, MorphBuffer* src, MorphBuffer* dst, VkDeviceSize size);