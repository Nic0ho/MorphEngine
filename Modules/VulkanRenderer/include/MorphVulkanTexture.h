#pragma once
#include <vulkan/vulkan.h>

namespace MorphVK
{
    struct VulkanTexture
    {
        VkImage m_image = VK_NULL_HANDLE;
        VkDeviceMemory m_mem = VK_NULL_HANDLE;
        VkImageView m_imageView = VK_NULL_HANDLE;
        VkSampler m_sampler = VK_NULL_HANDLE;

        void Destroy(VkDevice device)
        {
            if (m_sampler) vkDestroySampler(device, m_sampler, nullptr);
            if (m_imageView) vkDestroyImageView(device, m_imageView, nullptr);
            if (m_image) vkDestroyImage(device, m_image, nullptr);
            if (m_mem) vkFreeMemory(device, m_mem, nullptr);
        }
    };
}