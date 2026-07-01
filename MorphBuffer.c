#include "MorphBuffer.h"
#include "MorphTypes.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

// finding the right memory type on this GPU
// filter = which memory types are compatible (from vkGetBufferMemoryRequirements)
// properties = which flags we need (HOST_VISIBLE, DEVICE_LOCAL, etc.)
static u32 findMemoryType(VkPhysicalDevice physicalDevice, u32 filter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

    for (u32 i = 0; i < memProps.memoryTypeCount; i++)
    {
        bool typeCompatible = (filter & (1 << i));
        bool propsCompatible = (memProps.memoryTypes[i].propertyFlags & properties) == properties;
        
        if (typeCompatible && propsCompatible)
            return i;
    }

    printf("[BUFFER ERROR] Failed to find suitable memory type!\n");
    return UINT32_MAX;
}

bool morphBufferCreate(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, MorphBuffer* out)
{
    VkBufferCreateInfo bufferInfo = {0};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, NULL, &out->buffer) != VK_SUCCESS)
    {
        printf("[BUFFER ERROR] Failed to create buffer!\n");
        return false;
    }

    //ask Vulkan what memory requirements this buffer has
    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(device, out->buffer, &memReqs);

    u32 memTypeIndex = findMemoryType(physicalDevice, memReqs.memoryTypeBits, properties);
    if (memTypeIndex == UINT32_MAX)
        return false;

    VkMemoryAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = memTypeIndex;

    if (vkAllocateMemory(device, &allocInfo, NULL, &out->memory) != VK_SUCCESS)
    {
        printf("[BUFFER ERROR] Failed to allocate buffer memory!\n");
        return false;
    }

    vkBindBufferMemory(device, out->buffer, out->memory, 0);
    
    return true;
}

void morphBufferDestroy(VkDevice device, MorphBuffer* buf)
{
    vkDestroyBuffer(device, buf->buffer, NULL);
    vkFreeMemory(device, buf->memory, NULL);
}

//copy src buffer inro dst buffer using a temporary command buffer
void morphBufferCopy(VkDevice device, VkCommandPool pool, VkQueue queue, MorphBuffer* src, MorphBuffer* dst, VkDeviceSize size)
{
    //allocate a temporary one-time command buffer
    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool =  pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(device, &allocInfo, &cmd);

    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmd, &beginInfo);

    VkBufferCopy copyRegion = {0};
    copyRegion.size = size;

    vkCmdCopyBuffer(cmd, src->buffer, dst->buffer, 1, &copyRegion);

    vkEndCommandBuffer(cmd);

    //submit and wait for copy to complete
    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(device, pool, 1, &cmd);
}