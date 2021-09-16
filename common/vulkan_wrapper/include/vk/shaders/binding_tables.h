#pragma once
#ifdef NEKO_RAYTRACING
#include "vk/vk_include.h"

namespace neko::vk
{
class ShaderBindingTable : public Buffer
{
public:
	void Create(std::uint32_t handleCount);

    VkStridedDeviceAddressRegionKHR stridedDeviceAddressRegion {};
};

struct ShaderBindingTables
{
	void Create(std::uint32_t raygenHandleCount,
		std::uint32_t missHandleCount,
		std::uint32_t hitHandleCount);

	ShaderBindingTable raygen;
    ShaderBindingTable miss;
    ShaderBindingTable hit;
};
}
#endif
