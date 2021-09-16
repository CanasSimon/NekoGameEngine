#ifdef NEKO_RAYTRACING
#include "vk/shaders/binding_tables.h"

#include "vk/vk_utilities.h"

namespace neko::vk
{
void ShaderBindingTable::Create(std::uint32_t handleCount)
{
	const VkResources* vkObj = VkResources::Inst;
	const std::uint32_t shaderGroupHandleSize =
		vkObj->GetRayPipelineProperties().shaderGroupHandleSize;

	// Create buffer to hold all shader handles for the SBT
	Init(shaderGroupHandleSize * handleCount,
		VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Get the strided address to be used when dispatching the rays
    stridedDeviceAddressRegion = GetSbtEntryStridedDeviceAddressRegion(buffer_, handleCount);

    // Map persistent
	vkMapMemory(vkObj->device, memory_, 0, VK_WHOLE_SIZE, 0, &mapped);
}

void ShaderBindingTables::Create(std::uint32_t raygenHandleCount,
	std::uint32_t missHandleCount,
	std::uint32_t hitHandleCount)
{
	raygen.Create(raygenHandleCount);
	miss.Create(missHandleCount);
	hit.Create(hitHandleCount);
}
}
#endif