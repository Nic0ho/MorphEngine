#pragma once

#include <stdio.h>

#include <vulkan/vulkan.h>

#include "Common/MorphTypes.h"

namespace MorphVK
{
class VulkanQueue
{
public:
    VulkanQueue() {}
    ~VulkanQueue() {}

    void Init(VkDevice Device, VkSwapchainKHR SwapChain, u32 QueueFamily, u32 QueueIndex);

    void Destroy();
    u32 AcquireNextImage();

    void SubmitSync(VkCommandBuffer CmdBuf);
    void SubmitAsync(VkCommandBuffer CmdBuf);
    void Present(u32 ImageIndex);
    void WaitIdle();

private:
    void CreateSemaphores();

    VkDevice m_device = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
    VkQueue m_queue = VK_NULL_HANDLE;
    VkSemaphore m_renderCompleteSem;
    VkSemaphore m_presentCompleteSem;
};
}