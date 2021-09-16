#pragma once
#include "vk/particle/particle_instance.h"

namespace neko::vk
{
using ParticleInstanceIndex = std::size_t;
constexpr std::uint8_t kMaxParticleSystem = 128;
class ParticleCommandBuffer
{
public:
	ParticleCommandBuffer() = default;

    void Destroy();

	ParticleInstanceIndex AddParticleInstance(const MaterialId& material);
	void RemoveParticleInstance(ParticleInstanceIndex index);

    void DrawParticles(int index, const std::vector<ParticleDrawInfo>& particles);

	std::vector<std::unique_ptr<ParticleInstance>>& GetParticleDrawCommands();

private:
	std::vector<std::unique_ptr<ParticleInstance>> instanceBuffers_;
};
}
