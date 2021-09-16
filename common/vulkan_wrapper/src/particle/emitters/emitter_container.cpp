#include "vk/particle/emitters/emitter_container.h"

#include "vk/particle/particle_system.h"

namespace neko::vk
{
EmitterContainer::EmitterContainer() : type(EmitterType::POINT) {}

EmitterContainer::EmitterContainer(Emitter& newEmitter, EmitterType type) : type(type)
{
	switch (type)
	{
		//case EmitterType::CIRCLE: circle = reinterpret_cast<EmitterCircle&>(emitter); break;
		//case EmitterType::LINE: line = reinterpret_cast<EmitterLine&>(emitter); break;
		case EmitterType::POINT: emitter = reinterpret_cast<EmitterPoint&>(newEmitter); break;
		//case EmitterType::SPHERE: sphere = reinterpret_cast<EmitterSphere&>(emitter); break;
		//case EmitterType::CONE: cone = reinterpret_cast<EmitterCone&>(emitter); break;
		default:;
	}
}

EmitterContainer::~EmitterContainer()
{
	switch (type)
	{
		//case EmitterType::CIRCLE: break;
		//case EmitterType::LINE: break;
		case EmitterType::POINT: break;
		//case EmitterType::SPHERE: break;
		//case EmitterType::CONE: break;
		default:;
	}
}

bool EmitterContainer::operator==(const EmitterContainer& other) const
{
	if (type != other.type) return false;

	switch (type)
	{
		/*case EmitterType::CIRCLE:
			if (circle != other.circle) return false;
			break;
		case EmitterType::LINE:
			if (line != other.line) return false;
			break;*/
		case EmitterType::POINT:
			if (std::get<EmitterPoint>(emitter).point !=
				std::get<EmitterPoint>(other.emitter).point)
				return false;
			break;
		/*case EmitterType::SPHERE:
			if (sphere != other.sphere) return false;
			break;
		case EmitterType::CONE:
			if (cone != other.cone) return false;
			break;*/
		default:;
	}
	return true;
}

bool EmitterContainer::operator!=(const EmitterContainer& other) const { return !(*this == other); }

EmitterContainer::EmitterContainer(const EmitterContainer& other)
{
	type    = other.type;
	emitter = other.emitter;
}

EmitterContainer::EmitterContainer(EmitterContainer&& other) noexcept
{
	type    = other.type;
	emitter = other.emitter;
}

EmitterContainer& EmitterContainer::operator=(EmitterContainer&& other) noexcept
{
	type    = other.type;
	emitter = other.emitter;
	return *this;
}

Vec3f EmitterContainer::GetSpawnPos() const
{
    Vec3f pos;

	switch (type)
	{
		//case EmitterType::CIRCLE: pos = circle.GeneratePosition(); break;
		//case EmitterType::LINE: pos = line.GeneratePosition(); break;
		case EmitterType::POINT: pos = std::get<EmitterPoint>(emitter).point; break;
		//case EmitterType::SPHERE: pos = sphere.GeneratePosition(); break;
		//case EmitterType::CONE: pos = cone.GeneratePosition(); break;
		default:;
	}

	return pos;
}

Vec3f EmitterContainer::GetDirection(const Vec3f& spawnPos) const
{
    Vec3f direction;
	switch (type)
	{
		//case EmitterType::CIRCLE: direction = circle.GenerateDirection(spawnPos); break;
		//case EmitterType::LINE: direction = line.GenerateDirection(spawnPos); break;
		case EmitterType::POINT: direction = ParticleSystem::GenerateRandomUnitVector(); break;
		//case EmitterType::SPHERE: direction = sphere.GenerateDirection(spawnPos); break;
		//case EmitterType::CONE: direction = cone.GenerateDirection(spawnPos); break;
		default:;
	}

	return direction;
}
}