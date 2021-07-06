#include "vk/vk_utilities.h"

namespace neko::vk
{
std::uint32_t AlignedSize(std::uint32_t value, std::uint32_t alignment)
{
	return (value + alignment - 1) & ~(alignment - 1);
}

std::uint64_t GetBufferDeviceAddress(VkBuffer buffer)
{
	auto* vkObj = vk::VkResources::Inst;
	VkBufferDeviceAddressInfoKHR bufferDeviceAI {};
	bufferDeviceAI.sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	bufferDeviceAI.buffer = buffer;

	return VkResources::vkGetBufferDeviceAddressKHR(vkObj->device, &bufferDeviceAI);
}

VkStridedDeviceAddressRegionKHR GetSbtEntryStridedDeviceAddressRegion(
	VkBuffer buffer, std::uint32_t handleCount)
{
	const auto properties = VkResources::Inst->GetRayPipelineProperties();

	const uint32_t handleSizeAligned =
		AlignedSize(properties.shaderGroupHandleSize, properties.shaderGroupHandleAlignment);

	VkStridedDeviceAddressRegionKHR stridedDeviceAddressRegionKHR {};
	stridedDeviceAddressRegionKHR.deviceAddress = GetBufferDeviceAddress(buffer);
	stridedDeviceAddressRegionKHR.stride        = handleSizeAligned;
	stridedDeviceAddressRegionKHR.size          = handleCount * handleSizeAligned;
	return stridedDeviceAddressRegionKHR;
}

VkWriteDescriptorSet GenerateWriteDescriptorSet(VkDescriptorSet dstSet,
	VkDescriptorType type,
	uint32_t binding,
	VkDescriptorImageInfo* imageInfo,
	uint32_t descriptorCount)
{
	VkWriteDescriptorSet writeDescriptorSet {};
	writeDescriptorSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.dstSet          = dstSet;
	writeDescriptorSet.descriptorType  = type;
	writeDescriptorSet.dstBinding      = binding;
	writeDescriptorSet.pImageInfo      = imageInfo;
	writeDescriptorSet.descriptorCount = descriptorCount;
	return writeDescriptorSet;
}

VkWriteDescriptorSet GenerateWriteDescriptorSet(VkDescriptorSet dstSet,
	VkDescriptorType type,
	uint32_t binding,
	VkDescriptorBufferInfo* bufferInfo,
	uint32_t descriptorCount)
{
	VkWriteDescriptorSet writeDescriptorSet {};
	writeDescriptorSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.dstSet          = dstSet;
	writeDescriptorSet.descriptorType  = type;
	writeDescriptorSet.dstBinding      = binding;
	writeDescriptorSet.pBufferInfo     = bufferInfo;
	writeDescriptorSet.descriptorCount = descriptorCount;
	return writeDescriptorSet;
}

VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(VkDescriptorType type,
	VkShaderStageFlags stageFlags,
	uint32_t binding,
	uint32_t descriptorCount)
{
	VkDescriptorSetLayoutBinding setLayoutBinding {};
	setLayoutBinding.descriptorType  = type;
	setLayoutBinding.stageFlags      = stageFlags;
	setLayoutBinding.binding         = binding;
	setLayoutBinding.descriptorCount = descriptorCount;
	return setLayoutBinding;
}

Mat4f FromVkMat4(const VkTransformMatrixKHR& mat)
{
    Mat4f newMat = Mat4f::Identity;
    for (std::size_t x = 0; x < 3; ++x)
        for (std::size_t y = 0; y < 4; ++y)
            newMat[y][x] = mat.matrix[x][y];

	return newMat;
}

VkTransformMatrixKHR ToVkMat4(const Mat4f& mat)
{
    VkTransformMatrixKHR newMat {};
	for (std::size_t x = 0; x < 3; ++x)
		for (std::size_t y = 0; y < 4; ++y)
            newMat.matrix[x][y] = mat[y][x];

	return newMat;
}
}    // namespace neko::vk