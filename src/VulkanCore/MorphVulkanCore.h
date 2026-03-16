#pragma once

#include<vulkan/vulkan.h>

namespace MorphVK
{
class VulkanCore
{
public:
    VulkanCore();

    ~VulkanCore();

    void Init(const char* pAppName);

private:
    void CreateInstance(const char* pAppName);

    VkInstance m_instance = NULL;
};
}