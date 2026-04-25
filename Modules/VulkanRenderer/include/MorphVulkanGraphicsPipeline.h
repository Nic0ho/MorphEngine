#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "MorphVulkanSimpleMesh.h"
#include "MorphVulkanCore.h"

namespace MorphVK
{
class GraphicsPipeline
{
public:
    GraphicsPipeline(VkDevice Device, GLFWwindow* pWindow, VkRenderPass RenderPass, VkShaderModule vs, VkShaderModule fs, SimpleMesh* pMesh, int numImages, std::vector<BufferAndMemory>& UniformBuffers, int UniformDataSize);

    ~GraphicsPipeline();
    
    void Bind(VkCommandBuffer CmdBuf, int ImageIndex);
private:
    void CreateDescriptorPool(int NumImages);
    void AllocateDescriptorSets(int NumImages);
    void CreateDescriptorSets(int NumImages, const SimpleMesh* pMesh, std::vector<BufferAndMemory>& UniformBuffers, int UniformDataSize);
    void CreateDescriptorSetLayout(std::vector<BufferAndMemory>& UniformBuffers, int UniformDataSize, VulkanTexture* pTex);
    void UpdateDescriptorSets(int NumImages, const SimpleMesh* pMesh, std::vector<BufferAndMemory>& UniformBuffers, int UniformDataSize);

    VkDevice m_device = NULL;
    VkPipeline m_pipeline = NULL;
    VkPipelineLayout m_pipelineLayout = NULL;
    VkDescriptorPool m_descriptorPool;
    VkDescriptorSetLayout m_descriptorSetLayout;
    std::vector<VkDescriptorSet> m_descriptorSets;
};
}