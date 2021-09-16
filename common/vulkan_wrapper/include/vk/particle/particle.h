#pragma once
#include "graphics/color_gradient.h"

namespace neko::vk
{
struct ParticleDrawInfo
{
	bool operator==(const ParticleDrawInfo& other) const;
	bool operator!=(const ParticleDrawInfo& other) const;

	Vec3f position     = Vec3f::zero;     // 12
	Color4 colorOffset = Color::white;    // 12 -> 24

	Vec2f imageOffset1     = Vec2f::zero;    // 8 -> 32
	float imageBlendFactor = 0.0f;           // 4 -> 36

	Vec2f imageOffset2 = Vec2f::zero;    // 8 -> 44
	float alpha        = 1.0f;           // 4 -> 48
	float scale        = 1.0f;           // 4 -> 52 + offset -> 60
};

struct Particle
{
	Particle(const Vec3f& position,
		const Vec3f& velocity,
		float lifeLength,
		float scale,
		float gravityEffect,
		const Color4& colorOffset,
		const ColorGradient& colorOverLifetime);

	std::uint32_t numberOfRows = 1;
	Color4 originalColor       = Color::white;
	Color4 colorOffset         = Color::white;

	ColorGradient colorOverLifetime;

	Vec3f position = Vec3f::zero;
	Vec3f velocity = Vec3f::zero;
	Vec3f change   = Vec3f::zero;

	Vec2f imageOffset1 = Vec2f::zero;
	Vec2f imageOffset2 = Vec2f::zero;

	float lifetime      = 5.0f;
	float scale         = 1.0f;
	float gravityEffect = 1.0f;

	float elapsedTime      = 0.0f;
	float transparency     = 1.0f;
	float imageBlendFactor = 0.0f;
	float distanceToCamera = 0.0f;
};
}
