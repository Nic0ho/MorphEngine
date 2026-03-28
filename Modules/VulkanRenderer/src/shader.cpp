#include <stdio.h>
#ifdef _WIN64
#include <direct.h>
#endif
#include <vector>

#include <vulkan/vulkan.h>

#include "glslang/Include/glslang_c_interface.h"
#include "glslang/Public/resource_limits_c.h"
#include "SPIRV-Reflect/spirv_reflect.h"

#include "Common/MorphTypes.h"
#include "Common/MorphUtil.h"
#include "MorphVulkanUtil.h"
#include "MorphVulkanShader.h"


namespace MorphVK
{
struct Shader
{
    std::vector<u32> SPIRV;
    VkShaderModule ShaderModule = NULL;

    void Initialize(glslang_program_t* program)
    {
        size_t program_size = glslang_program_SPIRV_get_size(program);
        SPIRV.resize(program_size);
        glslang_program_SPIRV_get(program, SPIRV.data());
    }

    u32 GetPushConstantSize()
    {
        SpvReflectShaderModule SpvShaderModule;
        size_t Size = ARRAY_SIZE_IN_BYTES(SPIRV);

        SpvReflectResult result = spvReflectCreateShaderModule(Size, SPIRV.data(), &SpvShaderModule);
        if(result != SPV_REFLECT_RESULT_SUCCESS)
        {
            MORPH_ERROR("spvReflectCreateShaderModule error %d\n", result);
            exit(1);   
        }

        u32 PushConstantSize = 0;

        for(u32 i = 0; i < SpvShaderModule.push_constant_block_count; i++)
        {
            const SpvReflectBlockVariable& block = SpvShaderModule.push_constant_blocks[i];
            PushConstantSize = std::max(PushConstantSize, block.offset + block.size);
        }

        spvReflectDestroyShaderModule(&SpvShaderModule);

        return PushConstantSize;
    }
};

static void PrintShaderSource(const char* text)
{
    int line = 1;

    printf("\n(%3i) ", line);

    while(text && *text++)
    {
        if(*text == '\n') printf("\n(%3i) ", ++line);
        else if(*text == '\r') {}
        else printf("%c", *text);
    }

    printf("\n");
}

static bool CompileShader(VkDevice Device, glslang_stage_t Stage, const char* pShderCode, Shader& ShaderModule)
{
    glslang_input_t input =
    {
        .language = GLSLANG_SOURCE_GLSL,
        .stage = Stage,
        .client = GLSLANG_CLIENT_VULKAN,
        .client_version = GLSLANG_TARGET_VULKAN_1_4,
        .target_language = GLSLANG_TARGET_SPV,
        .target_language_version = GLSLANG_TARGET_SPV_1_6,
        .code = pShderCode,
        .default_version = 100,
        .default_profile = GLSLANG_NO_PROFILE,
        .force_default_version_and_profile = false,
        .forward_compatible = false,
        .messages = GLSLANG_MSG_DEFAULT_BIT,
        .resource = glslang_default_resource()
    };

    glslang_shader_t* shader = glslang_shader_create(&input);

    if(!glslang_shader_preprocess(shader, &input))
    {
        fprintf(stderr, "GLSL preprocessing failed\n");
        fprintf(stderr, "\n&s", glslang_shader_get_info_log(shader));
        fprintf(stderr, "\n&s", glslang_shader_get_info_debug_log(shader));
        PrintShaderSource(input.code);
        return 0;
    }

    if(!glslang_shader_parse(shader, &input))
    {
        fprintf(stderr, "GLSL parsing failed\n");
        fprintf(stderr, "\n&s", glslang_shader_get_info_log(shader));
        fprintf(stderr, "\n&s", glslang_shader_get_info_debug_log(shader));
        PrintShaderSource(glslang_shader_get_preprocessed_code(shader));
        return 0;
    }

    glslang_program_t* program = glslang_program_create();
    glslang_program_add_shader(program, shader);

    if(!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT))
    {
        fprintf(stderr, "GLSL linking failed\n");
        fprintf(stderr, "\n&s", glslang_program_get_info_log(program));
        fprintf(stderr, "\n&s", glslang_program_get_info_debug_log(program));
        return 0;
    }

    glslang_program_SPIRV_generate(program, Stage);

    ShaderModule.Initialize(program);

    const char* spirv_messages = glslang_program
}

VkShaderModule CreateShaderModuleFromText(VkDevice& Device, const char* pFilename)
{
    std::string Source;

    if(!ReadFile(pFilename, Source))
    { assert(0); }

    Shader ShaderModule;




}
}