#pragma once
#include <variant>

#include "vk/particle/emitters/emitter_point.h"

namespace neko::vk
{
struct EmitterContainer
{
	EmitterContainer();
	EmitterContainer(Emitter& emitter, EmitterType type);
	~EmitterContainer();

	bool operator==(const EmitterContainer& other) const;
	bool operator!=(const EmitterContainer& other) const;

	EmitterContainer(const EmitterContainer& other);
	EmitterContainer(EmitterContainer&& other) noexcept;
    EmitterContainer& operator=(const EmitterContainer& other) = default;
	EmitterContainer& operator=(EmitterContainer&& other) noexcept;

	[[nodiscard]] Vec3f GetSpawnPos() const;

	[[nodiscard]] Vec3f GetDirection(const Vec3f& spawnPos) const;

	EmitterType type;
    std::variant<EmitterPoint> emitter;
	/*union
	{
		//EmitterLine line {};
		EmitterPoint point {};
		//EmitterSphere sphere;
		//EmitterCircle circle;
		//EmitterCone cone;
	};*/
};
}
