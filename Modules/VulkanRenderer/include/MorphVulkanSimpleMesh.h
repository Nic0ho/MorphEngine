#pragma once

#include <vulkan/vulkan.h>

#include "Common/MorphTypes.h"
#include "MorphVulkanCore.h"

namespace MorphVK
{
struct SimpleMesh
{
    BufferAndMemory m_vb;
    size_t m_vertexBufferSize = 0;

    void Destroy(VkDevice Device)
    { m_vb.Destroy(Device); }
};
}