#pragma once
#include "math/basic.h"
#include "math/const.h"
#include "math/vector.h"

namespace neko::vk
{
enum class EmitterType : uint8_t
{
    POINT = 0,
	//CIRCLE = 0,
	//LINE,
	//POINT,
	//SPHERE,
	//CONE
};

class Emitter
{
public:
	Emitter() = default;
	virtual ~Emitter() = default;
};
}    // namespace neko::vk
