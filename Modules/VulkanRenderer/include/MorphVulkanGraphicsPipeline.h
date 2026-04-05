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
    
    void Bind(VkCommandBuffer CmdBuf, int ImageIndex);
private:
    void CreateDescriptorPool(int NumImages);
    void CreateDescriptorSets(int NumImages, const SimpleMesh* pMesh);
    void CreateDescriptorSetLayout();
    void AllocateDescriptorSets(int NumImages);
    void UpdateDescriptorSets(int NumImages, const SimpleMesh* pMesh);

    VkDevice m_device = NULL;
    VkPipeline m_pipeline = NULL;
    VkPipelineLayout m_pipelineLayout = NULL;
    VkDescriptorPool m_descriptorPool;
    VkDescriptorSetLayout m_descriptorSetLayout;
    std::vector<VkDescriptorSet> m_descriptorSets;
};
}