#pragma once
#include "sole.hpp"

#include "math/hash.h"

#include "vk/particle/emitters/emitter_container.h"
#include "vk/particle/particle.h"

namespace neko::vk
{
class ParticleSystem
{
public:
    ParticleSystem();
	~ParticleSystem() = default;

	bool operator==(const ParticleSystem& other) const;
	bool operator!=(const ParticleSystem& other) const;

	[[nodiscard]] Particle EmitParticle(Vec3f worldPos) const;

	bool UpdateLifetime(float dt);

	[[nodiscard]] static Vec3f GenerateRandomUnitVector();

	static float GenerateValue(float average, float errorPercent);

	Vec3f position = Vec3f::zero;

	//Control
	float timeEmit     = 0.0f;
	float lifetime     = 0.0f;
	float distanceEmit = 0.0f;
	Vec3f previousPos;

	//Particle System
	float duration      = 5.0f;
	bool looping        = true;
	float startDelay    = 0.0f;
	float minLifetime   = 10.0f;
	float maxLifetime   = 10.0f;
	float minSpeed      = 1.0f;
	float maxSpeed      = 1.0f;
	float minSize       = 1.0f;
	float maxSize       = 1.0f;
	float startRotation = 0.0f;    //TODO implement this.
	ColorGradient startColor;
	float gravityModifier = 0.0f;
	int maxParticles      = 1000;

	//Emission
	float rateOverTime     = 5.0f;
	float rateOverDistance = 0.0f;

	//Shape
	EmitterContainer emitter;
	float randomizeDirection = 0.0f;
	float randomizePosition  = 0.0f;

	//Color over lifetime
	ColorGradient colorOverLifetime;

	//Material
	sole::uuid materialID = sole::uuid();
};
}
