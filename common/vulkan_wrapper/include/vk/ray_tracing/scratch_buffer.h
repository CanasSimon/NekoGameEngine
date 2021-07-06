#pragma once

namespace neko::vk
{
struct ScratchBuffer
{
	ScratchBuffer(VkDeviceSize size);

    void Create(VkDeviceSize size);
	void Destroy() const;

    uint64_t deviceAddress = 0;
    VkBuffer handle        = VK_NULL_HANDLE;
    VkDeviceMemory memory  = VK_NULL_HANDLE;

private:
    PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR {};
};
}
