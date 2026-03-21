#include <vulkan/vulkan.h>

#include <stdlib.h>

#include "MorphVulkanUtil.h"
#include "Common/MorphUtil.h"

const char* GetDebugSeverityStr(VkDebugUtilsMessageSeverityFlagBitsEXT Severity)
{
    switch(Severity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            return "Verbose";

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            return "Info";
        
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            return "Warning";
        
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            return "Error";
        
        default:
            MORPH_ERROR("Invalid severity code %d\n", Severity);
            exit(1);
    }

    return "NO SUCH SEVERITY!";
}

const char* GetDebugType(VkDebugUtilsMessageTypeFlagsEXT Type)
{
    switch(Type)
    {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
            return "General";

        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
            return "Validation";

        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
            return "Performance";

        case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT:
            return "Device address binding";
        
        default:
            MORPH_ERROR("Invalid type code %d\n", Type);
            exit(1);
    }

    return "NO SUCH TYPE!";
}