#include "vk/ray_tracing/scratch_buffer.h"

namespace neko::vk
{
ScratchBuffer::ScratchBuffer(VkDeviceSize size)
{
	Create(size);
}

void ScratchBuffer::Create(VkDeviceSize size)
{
    auto* vkObj = vk::VkResources::Inst;
	vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(vkObj->device, "vkGetBufferDeviceAddressKHR"));

    // Buffer and memory
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    vkCreateBuffer(vkObj->device, &bufferCreateInfo, nullptr, &handle);

    VkMemoryRequirements memoryRequirements{};
    vkGetBufferMemoryRequirements(vkObj->device, handle, &memoryRequirements);

    VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
    memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = vkObj->gpu.GetMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkAllocateMemory(vkObj->device, &memoryAllocateInfo, nullptr, &memory);
    vkBindBufferMemory(vkObj->device, handle, memory, 0);

    // Buffer device address
    VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo {};
    bufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    bufferDeviceAddressInfo.buffer = handle;
    deviceAddress = vkGetBufferDeviceAddressKHR(vkObj->device, &bufferDeviceAddressInfo);
}

void ScratchBuffer::Destroy() const
{
	auto* vkObj = vk::VkResources::Inst;
	if (memory) vkFreeMemory(vkObj->device, memory, nullptr);
	if (handle) vkDestroyBuffer(vkObj->device, handle, nullptr);
}
}