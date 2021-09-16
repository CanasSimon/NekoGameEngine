#pragma once
#ifdef NEKO_RAYTRACING
#include "vk/vk_include.h"

namespace neko::vk
{
struct AccelerationStructure
{
    void Create(VkAccelerationStructureTypeKHR type,
                VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo);
    void Destroy() const;

	VkAccelerationStructureKHR handle {};
	std::uint64_t deviceAddress = 0;
	VkDeviceMemory memory {};
	VkBuffer buffer {};
};
}
#endif
