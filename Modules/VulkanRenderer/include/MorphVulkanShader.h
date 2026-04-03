#pragma once

#include <vulkan/vulkan.h>
namespace MorphVK
{
VkShaderModule CreateShaderModuleFromBinary(VkDevice device, const char* pFilename);
VkShaderModule CreateShaderModuleFromText(VkDevice device, const char* pFilename);
}