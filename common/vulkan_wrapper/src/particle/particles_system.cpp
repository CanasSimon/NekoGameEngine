#include "vk/particle/particles_system.h"

#include "sdl_engine/sdl_camera.h"

#include "vk/material/material_manager.h"

namespace neko::vk
{
ParticlesSystem::ParticlesSystem()
{
	particleSystems_.reserve(kBaseParticleSysNum);
    particlesDrawing_.reserve(kBaseParticleSysNum);
    particleInstanceIndexDrawing_.reserve(kBaseParticleSysNum);
    particleInstanceIndexRendering_.reserve(kBaseParticleSysNum);
}

void ParticlesSystem::Init() {}

inline Vec2f CalculateImageOffset(const std::int32_t index)
{
    const auto column = index % 1;
    const auto row    = index / 1;
    return Vec2f(static_cast<float>(column), static_cast<float>(row)) / 1.0f;
}

void ParticlesSystem::Update(seconds dt)
{
    const auto camPos = sdl::MultiCameraLocator::get().GetCamera(0).position;

	//Check life time and emit time
	for (std::size_t i = 0; i < particleSystems_.size(); i++)
	{
        auto particleSystem = particleSystems_[i];

		//Check lifetime
		if (!particleSystem.UpdateLifetime(dt.count()))
		{
			auto& fillingVector = particlesDrawing_[i];
			fillingVector.clear();
			continue;
		}

		auto& numberOfRows      = particles_[i].numberOfRows;
		auto& originalColor     = particles_[i].originalColor;
		auto& colorOffset       = particles_[i].colorOffset;
		auto& colorOverLifetime = particles_[i].colorOverLifetime;
		auto& position          = particles_[i].position;
		auto& velocity          = particles_[i].velocity;
		auto& imageOffset1      = particles_[i].imageOffset1;
		auto& imageOffset2      = particles_[i].imageOffset2;
		auto& lifetime          = particles_[i].lifetime;
		auto& scale             = particles_[i].scale;
		auto& elapsedTime       = particles_[i].elapsedTime;
		auto& transparency      = particles_[i].transparency;
		auto& imageBlendFactor  = particles_[i].imageBlendFactor;

		//Check Rate over Time Emit
		if (particleSystem.lifetime >= particleSystem.startDelay &&
			(particleSystem.lifetime <= particleSystem.duration || particleSystem.looping))
		{
			particleSystem.timeEmit += dt.count();

			const float rate = 1.0f / particleSystem.rateOverTime;

			if (particleSystem.timeEmit > rate)
			{
				const auto worldPos = particleSystem.position;
				while (particleSystem.timeEmit > rate)
				{
					auto particle = particleSystem.EmitParticle(worldPos);
					particleSystem.timeEmit -= rate;

					if (particles_[i].nbParticles >= particleSystem.maxParticles) continue;
					const std::size_t index = particles_[i].nbParticles++;
					if (numberOfRows.size() <= index)
					{
						const std::size_t newSize = std::min(
							index * 2 + 1, static_cast<std::size_t>(particleSystem.maxParticles));

						numberOfRows.resize(newSize);
						originalColor.resize(newSize);
						colorOffset.resize(newSize);
						colorOverLifetime.resize(newSize);
						position.resize(newSize);
						velocity.resize(newSize);
						imageOffset1.resize(newSize);
						imageOffset2.resize(newSize);
						lifetime.resize(newSize);
						scale.resize(newSize);
						elapsedTime.resize(newSize);
						transparency.resize(newSize);
						imageBlendFactor.resize(newSize);
					}

					numberOfRows[index]      = particle.numberOfRows;
					originalColor[index]     = particle.originalColor;
					colorOffset[index]       = particle.colorOffset;
					colorOverLifetime[index] = particle.colorOverLifetime;
					position[index]          = particle.position;
					velocity[index]          = particle.velocity;
					imageOffset1[index]      = particle.imageOffset1;
					imageOffset2[index]      = particle.imageOffset2;
					lifetime[index]          = particle.lifetime;
					scale[index]             = particle.scale;
					elapsedTime[index]       = 0;
					transparency[index]      = particle.transparency;
					imageBlendFactor[index]  = particle.imageBlendFactor;
				}
			}
		}

		//Check Rate Over Distance Emit
		if (particleSystem.rateOverDistance > 0.0f)
		{
			const auto worldPos = particleSystem.position;
			particleSystem.distanceEmit += Vec3f::GetDistance(worldPos, particleSystem.previousPos);
			particleSystem.previousPos = worldPos;

			if (particleSystem.distanceEmit > particleSystem.rateOverDistance)
			{
				while (particleSystem.distanceEmit > particleSystem.rateOverDistance)
				{
					auto particle = particleSystem.EmitParticle(worldPos);
					particleSystem.distanceEmit -= 1.0f / particleSystem.rateOverDistance;

					if (particles_[i].nbParticles >= particleSystem.maxParticles) continue;
					const std::size_t index = particles_[i].nbParticles++;

					if (numberOfRows.size() <= index)
					{
						const std::size_t newSize = std::min(
							index * 2 + 1, static_cast<std::size_t>(particleSystem.maxParticles));

						numberOfRows.resize(newSize);
						originalColor.resize(newSize);
						colorOffset.resize(newSize);
						colorOverLifetime.resize(newSize);
						position.resize(newSize);
						velocity.resize(newSize);
						imageOffset1.resize(newSize);
						imageOffset2.resize(newSize);
						lifetime.resize(newSize);
						scale.resize(newSize);
						elapsedTime.resize(newSize);
						transparency.resize(newSize);
						imageBlendFactor.resize(newSize);
					}

					numberOfRows[index]      = particle.numberOfRows;
					originalColor[index]     = particle.originalColor;
					colorOffset[index]       = particle.colorOffset;
					colorOverLifetime[index] = particle.colorOverLifetime;
					position[index]          = particle.position;
					velocity[index]          = particle.velocity;
					imageOffset1[index]      = particle.imageOffset1;
					imageOffset2[index]      = particle.imageOffset2;
					lifetime[index]          = particle.lifetime;
					scale[index]             = particle.scale;
					elapsedTime[index]       = 0;
					transparency[index]      = particle.transparency;
					imageBlendFactor[index]  = particle.imageBlendFactor;
				}
			}
		}

		particleSystems_[i] = particleSystem;
        if (particles_[i].nbParticles <= 0)
            continue;

		//Update particle system
		//Check alive
		const int size = particles_[i].nbParticles;
		for (std::size_t i = 0; i < size; ++i) elapsedTime[i] += dt.count();

		auto upperBound = particles_[i].nbParticles;
		for (auto j = size - 1; j >= 0; --j)
		{
			if (elapsedTime[j] < lifetime[j])
			{
				upperBound = j + 1;
				break;
			}
		}

		for (auto i = 0; i < upperBound; i++)
		{
			if (elapsedTime[i] >= lifetime[i])
			{
				//Swap dead and living particles
				const auto index = upperBound - 1;
				std::swap(numberOfRows[i], numberOfRows[index]);
				std::swap(originalColor[i], originalColor[index]);
				std::swap(colorOffset[i], colorOffset[index]);
				std::swap(colorOverLifetime[i], colorOverLifetime[index]);
				std::swap(position[i], position[index]);
				std::swap(velocity[i], velocity[index]);
				std::swap(imageOffset1[i], imageOffset1[index]);
				std::swap(imageOffset2[i], imageOffset2[index]);
				std::swap(lifetime[i], lifetime[index]);
				std::swap(scale[i], scale[index]);
				std::swap(elapsedTime[i], elapsedTime[index]);
				std::swap(transparency[i], transparency[index]);
				std::swap(imageBlendFactor[i], imageBlendFactor[index]);
				for (auto j = upperBound - 1; j >= 0; --j)
				{
					if (elapsedTime[j] < lifetime[j])
					{
						upperBound = j + 1;
						break;
					}
				}
			}
		}

		particles_[i].nbParticles = upperBound;
		if (upperBound <= 0) continue;

		//Update velocity
        const float gravityFactor = -10.0f * particleSystem.gravityModifier * dt.count();
        for (int j = 0; j < upperBound; j++) velocity[j].y += gravityFactor;

        //Update position
        for (int j = 0; j < upperBound; j++) position[j] += velocity[j] * dt.count();

        //Update
		for (int j = 0; j < upperBound; j++)
		{
			//Color
			const auto lifeFactor = elapsedTime[j] / lifetime[j];
			colorOffset[j] =
				originalColor[j] * particleSystem.colorOverLifetime.GetColorAt(lifeFactor);

			transparency[j] = colorOffset[j].a;

			const auto stageCount       = static_cast<std::int32_t>(pow(1, 2));
			const auto atlasProgression = 0 * lifeFactor * stageCount; //TODO replace 0 with stage cycles
			const auto index1           = static_cast<int32_t>(std::floor(atlasProgression));
			const auto index2           = index1 < stageCount - 1 ? index1 + 1 : index1;

			imageBlendFactor[j] = std::fmod(atlasProgression, 1.0f);
			imageOffset1[j]     = CalculateImageOffset(index1);
			imageOffset2[j]     = CalculateImageOffset(index2);
		}

		//Sort particles
		std::vector<std::pair<float, std::size_t>> sortedIndex(upperBound);
		for (int j = 0; j < upperBound; j++)
		{
			const auto distance = Vec3f::GetDistanceManhattan(camPos, position[j]);
			sortedIndex[j]      = std::pair<float, std::size_t>(distance, j);
		}

		std::sort(sortedIndex.begin(),
			sortedIndex.end(),
			[](const std::pair<float, std::size_t>& d1, const std::pair<float, std::size_t>& d2)
			{ return d1 < d2; });

		//Prepare vector for receiving drawing data
		auto& fillingVector = particlesDrawing_[i];
		fillingVector.clear();
		fillingVector.resize(upperBound);

		//Create drawing info
		for (std::size_t particleIndex = 0; particleIndex < upperBound; particleIndex++)
		{
			auto index                            = sortedIndex[particleIndex].second;
			fillingVector[upperBound - index - 1] = ParticleDrawInfo {position[index],
				colorOffset[index],
				imageOffset1[index],
				imageBlendFactor[index],
				imageOffset2[index],
				transparency[index],
				scale[index]};
		}
	}

	OnCulling();
}

void ParticlesSystem::Destroy()
{
	particleSystems_.clear();

	particleInstanceIndexDrawing_.clear();
	particleInstanceIndexRendering_.clear();

	particlesDrawing_.clear();
	particles_.clear();
}

void ParticlesSystem::OnCulling()
{
	for (std::size_t i = 0; i < particlesDrawing_.size(); i++)
	{
		VkResources::Inst->particleCommandBuffer.DrawParticles(
			particleInstanceIndexRendering_[i], particlesDrawing_[i]);
	}
}

void ParticlesSystem::AddSystem()
{
	AddSystem(ParticleSystem());
}

void ParticlesSystem::AddSystem(const ParticleSystem& particlesSystem)
{
    auto& particleSystem = particleSystems_.emplace_back(particlesSystem);
    particleSystem.previousPos = particleSystem.position;

	particlesDrawing_.emplace_back();
	particles_.emplace_back();
	particleInstanceIndexDrawing_.emplace_back(
		VkResources::Inst->particleCommandBuffer.AddParticleInstance(particleSystem.materialID));
}
}    // namespace neko::vk