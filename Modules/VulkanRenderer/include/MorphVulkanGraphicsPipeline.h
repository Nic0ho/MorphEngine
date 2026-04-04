#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "MorphVulkanSimpleMesh.h"

namespace MorphVK
{
class GraphicsPipeline
{
public:
    GraphicsPipeline(VkDevice Device, GLFWwindow* pWindow, VkRenderPass, VkShaderModule vs, VkShaderModule fs, SimpleMesh* pMesh, int numImages);

    ~GraphicsPipeline();
    
    void Bind(VkCommandBuffer CmdBuf);
private:
    VkDevice m_device = NULL;
    VkPipeline m_pipeline = NULL;
    VkPipelineLayout m_pipelineLayout = NULL;
};
}