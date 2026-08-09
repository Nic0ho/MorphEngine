#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;

layout(push_constant) uniform PushConstants
{
    mat4 transform;
    vec2 uvOffset;
    vec2 uvScale;
} pc;

void main()
{
    gl_Position = pc.transform * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
    fragUV = pc.uvOffset + inUV * pc.uvScale;
}