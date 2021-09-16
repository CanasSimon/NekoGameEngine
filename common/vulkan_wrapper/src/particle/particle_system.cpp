#include "vk/particle/particle_system.h"

#include "math/basic.h"
#include "math/const.h"

namespace neko::vk
{
ParticleSystem::ParticleSystem()
{
	materialID = sole::rebuild(666666, 999999);
}

bool ParticleSystem::operator==(const ParticleSystem& other) const
{
    return rateOverTime == other.rateOverTime
           && minSpeed == other.minSpeed
           && gravityModifier == other.gravityModifier
           && startColor == other.startColor
           && randomizeDirection == other.randomizeDirection
           && maxSpeed == other.maxSpeed
           && minLifetime == other.minLifetime
           && maxLifetime == other.maxLifetime
           && minSize == other.minSize
           && maxSize == other.maxSize
           && timeEmit == other.timeEmit
           && duration == other.duration
           && looping == other.looping
           && startDelay == other.startDelay
           && lifetime == other.lifetime
           && minSize == other.minSize
           && maxSize == other.maxSize
           && startRotation == other.startRotation
           && maxParticles == other.maxParticles
           && rateOverDistance == other.rateOverDistance
           && randomizePosition == other.randomizePosition
           && emitter == other.emitter
           && colorOverLifetime == other.colorOverLifetime
           && materialID == other.materialID;
}

bool ParticleSystem::operator!=(const ParticleSystem& other) const { return !(*this == other); }

Particle ParticleSystem::EmitParticle(Vec3f worldPos) const
{
	Vec3f spawnPos = emitter.GetSpawnPos();
	Vec3f velocity = emitter.GetDirection(spawnPos);

	spawnPos += worldPos + GenerateRandomUnitVector() * randomizePosition;

	velocity = velocity.Normalized();
	velocity *= RandomRange(minSpeed, maxSpeed);
	const auto scale      = RandomRange(minSize, maxSize);
	const auto lifeLength = RandomRange(minLifetime, maxLifetime);

	return Particle {spawnPos,
		velocity,
		lifeLength,
		scale,
		gravityModifier,
		startColor.GetColorAt(RandomRange(0.0f, 1.0f)),
		colorOverLifetime};
}

bool ParticleSystem::UpdateLifetime(float dt)
{
	lifetime += dt;
	if (!looping && duration + maxLifetime <= lifetime)
	{
		lifetime = 0;
		return false;
	}

	return true;
}

Vec3f ParticleSystem::GenerateRandomUnitVector()
{
	const float theta                = RandomRange(0.0f, 1.0f) * 2.0f * PI;
	const float z                    = RandomRange(0.0f, 1.0f) * 2.0f - 1.0f;
	const float rootOneMinusZSquared = Sqrt(1.0f - z * z);
	const float x                    = rootOneMinusZSquared * Cos(radian_t(theta));
	const float y                    = rootOneMinusZSquared * Sin(radian_t(theta));
	return {x, y, z};
}

float ParticleSystem::GenerateValue(float average, float errorPercent)
{
	const auto error = RandomRange(-1.0f, 1.0f) * errorPercent;
	return average + (average * error);
}
}