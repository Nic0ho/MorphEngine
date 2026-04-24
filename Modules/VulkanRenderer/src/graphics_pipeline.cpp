#include <stdio.h>

#include "Common/MorphTypes.h"
#include "Common/MorphUtil.h"
#include "MorphVulkanCore.h"
#include "MorphVulkanSimpleMesh.h"
#include "MorphVulkanUtil.h"
#include "MorphVulkanGraphicsPipeline.h"

namespace MorphVK
{

GraphicsPipeline::GraphicsPipeline(VkDevice Device, GLFWwindow* pWindow,
                                   VkRenderPass RenderPass,
                                   VkShaderModule vs, VkShaderModule fs,
                                   SimpleMesh* pMesh, int numImages,
                                   std::vector<BufferAndMemory>& UniformBuffers,
                                   int UniformDataSize)
{
    m_device = Device;

    if (pMesh) CreateDescriptorSets(numImages, pMesh, UniformBuffers, UniformDataSize);

    VkPipelineShaderStageCreateInfo ShaderStageCreateInfo[2] =
    {
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vs,
            .pName  = "main"
        },
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fs,
            .pName  = "main"
        }
    };

    VkPipelineVertexInputStateCreateInfo VertexInputInfo =
    { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

    VkPipelineInputAssemblyStateCreateInfo PipelineIACreateInfo =
    {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    int WindowWidth, WindowHeight;
    glfwGetFramebufferSize(pWindow, &WindowWidth, &WindowHeight);

    VkViewport VP =
    {
        .x        = 0.0f,
        .y        = 0.0f,
        .width    = (float)WindowWidth,
        .height   = (float)WindowHeight,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D Scissor =
    {
        .offset = { .x = 0, .y = 0 },
        .extent = { .width = (u32)WindowWidth, .height = (u32)WindowHeight }
    };

    VkPipelineViewportStateCreateInfo VPCreateInfo =
    {
        .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports    = &VP,
        .scissorCount  = 1,
        .pScissors     = &Scissor
    };

    VkPipelineRasterizationStateCreateInfo RastCreateInfo =
    {
        .sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode    = VK_CULL_MODE_NONE,
        .frontFace   = VK_FRONT_FACE_CLOCKWISE,
        .lineWidth   = 1.0f
    };

    VkPipelineMultisampleStateCreateInfo PipelineMSCreateInfo =
    {
        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable   = VK_FALSE,
        .minSampleShading      = 1.0f
    };

    VkPipelineColorBlendAttachmentState BlendAttachState =
    {
        .blendEnable    = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineColorBlendStateCreateInfo BlendCreateInfo =
    {
        .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable   = VK_FALSE,
        .logicOp         = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments    = &BlendAttachState
    };

    VkPipelineLayoutCreateInfo LayoutInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };

    if (pMesh && pMesh->m_vb.m_buffer)
    {
        LayoutInfo.setLayoutCount = 1;
        LayoutInfo.pSetLayouts    = &m_descriptorSetLayout;
    }
    else
    {
        LayoutInfo.setLayoutCount = 0;
        LayoutInfo.pSetLayouts    = NULL;
    }

    VkResult res = vkCreatePipelineLayout(m_device, &LayoutInfo, NULL, &m_pipelineLayout);
    CHECK_VK_RESULT(res, "vkCreatePipelineLayout\n");

    VkGraphicsPipelineCreateInfo PipelineInfo =
    {
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount          = ARRAY_SIZE_IN_ELEMENTS(ShaderStageCreateInfo),
        .pStages             = &ShaderStageCreateInfo[0],
        .pVertexInputState   = &VertexInputInfo,
        .pInputAssemblyState = &PipelineIACreateInfo,
        .pViewportState      = &VPCreateInfo,
        .pRasterizationState = &RastCreateInfo,
        .pMultisampleState   = &PipelineMSCreateInfo,
        .pColorBlendState    = &BlendCreateInfo,
        .layout              = m_pipelineLayout,
        .renderPass          = RenderPass,
        .subpass             = 0,
        .basePipelineHandle  = VK_NULL_HANDLE,
        .basePipelineIndex   = -1
    };

    res = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &PipelineInfo, NULL, &m_pipeline);
    CHECK_VK_RESULT(res, "vkCreateGraphicsPipelines\n");

    printf("Graphics pipeline created\n");
}

GraphicsPipeline::~GraphicsPipeline()
{
    vkDestroyPipeline(m_device, m_pipeline, NULL);
    vkDestroyPipelineLayout(m_device, m_pipelineLayout, NULL);
    if (m_descriptorSetLayout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, NULL);
    if (m_descriptorPool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(m_device, m_descriptorPool, NULL);
}

void GraphicsPipeline::Bind(VkCommandBuffer CmdBuf, int ImageIndex)
{
    vkCmdBindPipeline(CmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    if (!m_descriptorSets.empty())
    {
        vkCmdBindDescriptorSets(CmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_pipelineLayout, 0, 1,
                                &m_descriptorSets[ImageIndex], 0, NULL);
    }
}

void GraphicsPipeline::CreateDescriptorSets(int NumImages, const SimpleMesh* pMesh,
                                            std::vector<BufferAndMemory>& UniformBuffers,
                                            int UniformDataSize)
{
    CreateDescriptorPool(NumImages);
    CreateDescriptorSetLayout(UniformBuffers, UniformDataSize);
    AllocateDescriptorSets(NumImages);
    UpdateDescriptorSets(NumImages, pMesh, UniformBuffers, UniformDataSize);
}

void GraphicsPipeline::CreateDescriptorPool(int NumImages)
{
    std::vector<VkDescriptorPoolSize> PoolSizes =
    {
        {
            .type            = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = (u32)NumImages
        },
        {
            .type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = (u32)NumImages
        }
    };

    VkDescriptorPoolCreateInfo PoolInfo =
    {
        .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags         = 0,
        .maxSets       = (u32)NumImages,
        .poolSizeCount = (u32)PoolSizes.size(),
        .pPoolSizes    = PoolSizes.data()
    };

    VkResult res = vkCreateDescriptorPool(m_device, &PoolInfo, NULL, &m_descriptorPool);
    CHECK_VK_RESULT(res, "vkCreateDescriptorPool");
    printf("Descriptor pool created\n");
}

void GraphicsPipeline::CreateDescriptorSetLayout(std::vector<BufferAndMemory>& UniformBuffers,
                                                 int UniformDataSize)
{
    std::vector<VkDescriptorSetLayoutBinding> LayoutBindings;

    LayoutBindings.push_back(VkDescriptorSetLayoutBinding{
        .binding         = 0,
        .descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags      = VK_SHADER_STAGE_VERTEX_BIT
    });

    if (!UniformBuffers.empty())
    {
        LayoutBindings.push_back(VkDescriptorSetLayoutBinding{
            .binding         = 1,
            .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags      = VK_SHADER_STAGE_VERTEX_BIT
        });
    }

    VkDescriptorSetLayoutCreateInfo LayoutInfo =
    {
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext        = NULL,
        .flags        = 0,
        .bindingCount = (u32)LayoutBindings.size(),
        .pBindings    = LayoutBindings.data()
    };

    VkResult res = vkCreateDescriptorSetLayout(m_device, &LayoutInfo, NULL, &m_descriptorSetLayout);
    CHECK_VK_RESULT(res, "vkCreateDescriptorSetLayout");
}

void GraphicsPipeline::AllocateDescriptorSets(int NumImages)
{
    std::vector<VkDescriptorSetLayout> Layouts(NumImages, m_descriptorSetLayout);

    VkDescriptorSetAllocateInfo AllocInfo =
    {
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext              = NULL,
        .descriptorPool     = m_descriptorPool,
        .descriptorSetCount = (u32)NumImages,
        .pSetLayouts        = Layouts.data()
    };

    m_descriptorSets.resize(NumImages);
    VkResult res = vkAllocateDescriptorSets(m_device, &AllocInfo, m_descriptorSets.data());
    CHECK_VK_RESULT(res, "AllocateDescriptorSets");
}

void GraphicsPipeline::UpdateDescriptorSets(int NumImages, const SimpleMesh* pMesh,
                                            std::vector<BufferAndMemory>& UniformBuffers,
                                            int UniformDataSize)
{
    VkDescriptorBufferInfo BufferInfo_VB =
    {
        .buffer = pMesh->m_vb.m_buffer,
        .offset = 0,
        .range  = pMesh->m_vertexBufferSize
    };

    std::vector<VkWriteDescriptorSet> WriteDescriptorSet;

    for (int i = 0; i < NumImages; i++)
    {
        WriteDescriptorSet.push_back(VkWriteDescriptorSet{
            .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet          = m_descriptorSets[i],
            .dstBinding      = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .pBufferInfo     = &BufferInfo_VB
        });

        if (!UniformBuffers.empty())
        {
            VkDescriptorBufferInfo BufferInfo_Uniform =
            {
                .buffer = UniformBuffers[i].m_buffer,
                .offset = 0,
                .range  = (VkDeviceSize)UniformDataSize
            };

            WriteDescriptorSet.push_back(VkWriteDescriptorSet{
                .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet          = m_descriptorSets[i],
                .dstBinding      = 1,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo     = &BufferInfo_Uniform
            });
        }
    }

    vkUpdateDescriptorSets(m_device, (u32)WriteDescriptorSet.size(),
                           WriteDescriptorSet.data(), 0, NULL);
}
}