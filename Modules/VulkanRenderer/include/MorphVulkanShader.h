#pragma once

namespace MorphVK
{
VkShaderModule CreateShaderModuleFromBinary(VkDevice& device, const char* pFilename);
VkShaderModule CreateShaderModuleFromText(VkDevice& device, const char* pFilename);
}