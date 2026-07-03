#pragma once
#include "MorphTypes.h"
#include <vulkan/vulkan.h>

typedef struct // ------- MORPH BUFFER ------------
{
    VkBuffer       buffer;
    VkDeviceMemory memory;
} MorphBuffer;

bool morphBufferCreate(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, MorphBuffer* out);
void morphBufferDestroy(VkDevice device, MorphBuffer* buf);
void morphBufferCopy(VkDevice device, VkCommandPool pool, VkQueue queue, MorphBuffer* src, MorphBuffer* dst, VkDeviceSize size);

typedef struct // ------- MORPH TEXTURE ------------
{
    VkImage        image;
    VkDeviceMemory memory;
    VkImageView    view;
    VkSampler      sampler;
} MorphTexture;

bool morphTextureLoad(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool pool, VkQueue queue, const char* path, MorphTexture* out);
void morphTextureDestroy(VkDevice device, MorphTexture* tex);