#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout(location = 0) in vec3 inPosition;
layout(location = 2) in vec2 inUV;

layout(location = 5) in mat4 inModelMatrix;
layout(location = 9) in vec4 inColorOffset;
layout(location = 10) in vec4 inOffsets;
layout(location = 11) in vec3 inBlend;

layout(location = 0) out vec2 outCoords1;
layout(location = 1) out vec2 outCoords2;
layout(location = 2) out vec4 outColorOffset;
layout(location = 3) out float outBlendFactor;
layout(location = 4) out float outAlpha;

layout(binding = 0) uniform UboScene
{
    mat4 view;
    mat4 proj;
} scene;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    gl_Position = scene.proj * scene.view * inModelMatrix * vec4(inPosition, 1.0f);

    vec2 uv = inUV / inBlend.z;

    outColorOffset = inColorOffset;
    outCoords1 = uv + inOffsets.xy;
    outCoords2 = uv + inOffsets.zw;

    outBlendFactor = inBlend.x;
    outAlpha = inBlend.y;
}

