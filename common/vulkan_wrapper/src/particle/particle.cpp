#include "vk/particle/particle.h"

namespace neko::vk
{
bool ParticleDrawInfo::operator==(const ParticleDrawInfo& other) const
{
    return position == other.position
           && scale == other.scale
           && colorOffset == other.colorOffset
           && imageOffset1 == other.imageOffset1
           && imageBlendFactor == other.imageBlendFactor
           && imageOffset2 == other.imageOffset2
           && alpha == other.alpha;
}

bool ParticleDrawInfo::operator!=(const ParticleDrawInfo& other) const { return !(*this == other); }

Particle::Particle(const Vec3f& position,
	const Vec3f& velocity,
	float lifeLength,
	float scale,
	float gravityEffect,
	const Color4& colorOffset,
	const ColorGradient& colorOverLifetime)
   : originalColor(colorOffset),
	 colorOverLifetime(colorOverLifetime),
	 position(position),
	 velocity(velocity),
	 lifetime(lifeLength),
	 scale(scale),
	 gravityEffect(gravityEffect)
{}
}    // namespace neko::vk