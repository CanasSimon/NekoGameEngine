#ifdef NEKO_RAYTRACING
#include "vk/ray_tracing/acceleration_structure.h"

#include "vk/ray_tracing/scratch_buffer.h"
#include "vk/vk_resources.h"

namespace neko::vk
{
void AccelerationStructure::Create(
	VkAccelerationStructureTypeKHR type, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo)
{
    auto* vkObj = vk::VkResources::Inst;

    // Buffer and memory
    VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size  = buildSizeInfo.accelerationStructureSize;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    vkCreateBuffer(vkObj->device, &bufferCreateInfo, nullptr, &buffer);

    VkMemoryRequirements memoryRequirements{};
    vkGetBufferMemoryRequirements(vkObj->device, buffer, &memoryRequirements);

    VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
    memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

    VkMemoryAllocateInfo memoryAllocateInfo{};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = vkObj->gpu.GetMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkAllocateMemory(vkObj->device, &memoryAllocateInfo, nullptr, &memory);
    vkBindBufferMemory(vkObj->device, buffer, memory, 0);

    // Acceleration structure
    VkAccelerationStructureCreateInfoKHR accelerationStructureCreate_info{};
    accelerationStructureCreate_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    accelerationStructureCreate_info.buffer = buffer;
    accelerationStructureCreate_info.size = buildSizeInfo.accelerationStructureSize;
    accelerationStructureCreate_info.type = type;
    VkResources::vkCreateAccelerationStructureKHR(vkObj->device, &accelerationStructureCreate_info, nullptr, &handle);

    // AS device address
    VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
    accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
    accelerationDeviceAddressInfo.accelerationStructure = handle;
    deviceAddress = VkResources::vkGetAccelerationStructureDeviceAddressKHR(vkObj->device, &accelerationDeviceAddressInfo);
}

void AccelerationStructure::Destroy() const
{
    auto* vkObj = vk::VkResources::Inst;
    vkFreeMemory(vkObj->device, memory, nullptr);
    vkDestroyBuffer(vkObj->device, buffer, nullptr);
    VkResources::vkDestroyAccelerationStructureKHR(vkObj->device, handle, nullptr);
}
}
#endif