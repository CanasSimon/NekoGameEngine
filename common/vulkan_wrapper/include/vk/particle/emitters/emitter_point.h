#pragma once
#include "vk/particle/emitters/emitter.h"

namespace neko::vk
{
class EmitterPoint : public Emitter
{
public:
    EmitterPoint() = default;
	explicit EmitterPoint(const Vec3f& pos) : point(pos) {}
	~EmitterPoint() override = default;

	Vec3f point;
};
}
