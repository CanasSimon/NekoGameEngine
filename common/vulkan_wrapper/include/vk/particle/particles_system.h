#pragma once
#include "engine/system.h"

#include "vk/particle/particle_instance.h"
#include "vk/particle/particle_system.h"

namespace neko::vk
{
constexpr std::uint8_t kBaseParticleSysNum = 255;
class ParticlesSystem final : public SystemInterface
{
public:
	explicit ParticlesSystem();
	~ParticlesSystem() override = default;

	void Init() override;
	void Update(seconds dt) override;
	void Destroy() override;

	void AddSystem();
	void AddSystem(const ParticleSystem& particlesSystem);

private:
	void OnCulling();

	//Entities
	std::vector<ParticleSystem> particleSystems_;

	//Drawing data
	std::vector<std::vector<ParticleDrawInfo>> particlesDrawing_;

	//Index of particle instance
	std::vector<int> particleInstanceIndexDrawing_;
	std::vector<int> particleInstanceIndexRendering_;

	//Particles
	struct Particle
	{
		int nbParticles;

		std::vector<std::uint32_t> numberOfRows;
		std::vector<Color4> originalColor;
		std::vector<Color4> colorOffset;

		std::vector<ColorGradient> colorOverLifetime;

		std::vector<Vec3f> position;
		std::vector<Vec3f> velocity;

		std::vector<Vec2f> imageOffset1;
		std::vector<Vec2f> imageOffset2;

		std::vector<float> lifetime;
		std::vector<float> scale;

		std::vector<float> elapsedTime;
		std::vector<float> transparency;
		std::vector<float> imageBlendFactor;
	};

	//Current particles
	std::vector<Particle> particles_;
};
}
