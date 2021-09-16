#include "vk/commands/particle_command_buffer.h"

namespace neko::vk
{
void ParticleCommandBuffer::Destroy()
{
	for (auto& particleInstance : instanceBuffers_) particleInstance.reset();
}

ParticleInstanceIndex ParticleCommandBuffer::AddParticleInstance(const MaterialId& material)
{
    for (size_t i = 0; i < instanceBuffers_.size(); i++)
		if (instanceBuffers_[i]->GetMaterialId() == material)
			return i;

	instanceBuffers_.push_back(nullptr);
	instanceBuffers_.back() = std::make_unique<ParticleInstance>(material);
	return instanceBuffers_.size() - 1;
}

void ParticleCommandBuffer::RemoveParticleInstance(ParticleInstanceIndex index)
{
    instanceBuffers_[index] = nullptr;
}

void ParticleCommandBuffer::DrawParticles(int index, const std::vector<ParticleDrawInfo>& particles)
{
    instanceBuffers_[index]->Update(particles);
}

std::vector<std::unique_ptr<ParticleInstance>>& ParticleCommandBuffer::GetParticleDrawCommands()
{
	return instanceBuffers_;
}
}