#pragma once
#include "math/matrix.h"

#include "vk/vk_include.h"

namespace neko::vk
{
std::uint32_t AlignedSize(std::uint32_t value, std::uint32_t alignment);

#ifdef NEKO_RAYTRACING
std::uint64_t GetBufferDeviceAddress(VkBuffer buffer);
VkStridedDeviceAddressRegionKHR GetSbtEntryStridedDeviceAddressRegion(
	VkBuffer buffer, std::uint32_t handleCount);
#endif

VkWriteDescriptorSet GenerateWriteDescriptorSet(VkDescriptorSet dstSet,
	VkDescriptorType type,
	uint32_t binding,
	VkDescriptorImageInfo* imageInfo,
	uint32_t descriptorCount = 1);
VkWriteDescriptorSet GenerateWriteDescriptorSet(
    VkDescriptorSet dstSet,
    VkDescriptorType type,
    uint32_t binding,
    VkDescriptorBufferInfo* bufferInfo,
    uint32_t descriptorCount = 1);

VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(
    VkDescriptorType type,
    VkShaderStageFlags stageFlags,
    uint32_t binding,
    uint32_t descriptorCount = 1);

Mat4f FromVkMat4(const VkTransformMatrixKHR& mat);
VkTransformMatrixKHR ToVkMat4(const Mat4f& mat);
}    // namespace neko::vk
