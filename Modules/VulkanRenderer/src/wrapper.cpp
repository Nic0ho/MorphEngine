#include "MorphVulkanUtil.h"
#include "MorphVulkanWrapper.h"
#include <cstddef>

namespace MorphVK
{
void BeginCommandBuffer(VkCommandBuffer CommandBuffer, VkCommandBufferUsageFlags UsageFlags)
{
    VkCommandBufferBeginInfo BeginInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = NULL,
        .flags = UsageFlags,
        .pInheritanceInfo = NULL
    };

    VkResult res = vkBeginCommandBuffer(CommandBuffer, &BeginInfo);
    CHECK_VK_RESULT(res, "vkBeginCommandBuffer\n");
}

VkSemaphore CreateSemaphore(VkDevice Device)
{
    VkSemaphoreCreateInfo CreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0
    };

    VkSemaphore Semaphore;
    VkResult res = vkCreateSemaphore(Device, &CreateInfo, NULL, &Semaphore);
    CHECK_VK_RESULT(res, "vkCreateSemaphore");
    return Semaphore;
}
}