#pragma once

#include <stdio.h>
#include <vector>
#include <cstdlib>
#include <vulkan/vulkan.h>
#include "Common/MorphTypes.h"

typedef unsigned int uint;

#define CHECK_VK_RESULT(res, msg)                                                        \
    if(res != VK_SUCCESS)                                                                \
    {                                                                                    \
        fprintf(stderr, "Error in %s:%d - %s, code %x\n", __FILE__, __LINE__, msg, res); \
        exit(1);                                                                         \
    }

#define MORPH_ERROR(msg, ...) fprintf(stderr, msg, ##__VA_ARGS__)

namespace MorphVK
{
const char* GetDebugSeverityStr(VkDebugUtilsMessageSeverityFlagBitsEXT Severity);
const char* GetDebugType(VkDebugUtilsMessageTypeFlagsEXT Type);

int GetBytesPerTexFormat(VkFormat Format);
bool HasStencilComponent(VkFormat Format);

VkFormat FindSupportedFormat(VkPhysicalDevice Device, const std::vector<VkFormat>& Candidates, VkImageTiling Tiling, VkFormatFeatureFlags Features);
}
