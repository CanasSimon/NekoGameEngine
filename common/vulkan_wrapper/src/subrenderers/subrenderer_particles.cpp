#include "vk/subrenderers/subrenderer_particles.h"

namespace neko::vk
{
SubrendererParticles::SubrendererParticles(const PipelineStage& pipelineStage)
    : RenderPipeline(pipelineStage)
{}

void SubrendererParticles::Render(const CommandBuffer& commandBuffer)
{
    const auto camera = sdl::MultiCameraLocator::get().GetCamera(0);
    const Mat4f view = camera.GenerateViewMatrix();
    Mat4f proj       = camera.GenerateProjectionMatrix();
    proj[1][1] *= -1.0f;

    uniformScene_.Push(kViewHash, view);
    uniformScene_.Push(kProjHash, proj);

    auto& particleCommandBuffer = VkResources::Inst->particleCommandBuffer;
    for (auto& particle : particleCommandBuffer.GetParticleDrawCommands())
	{
        Material& material = MaterialManagerLocator::get().GetMaterial(particle->GetMaterialId());
		particle->CmdRender(commandBuffer, uniformScene_, material);
	}
}

void SubrendererParticles::Destroy() const
{
    uniformScene_.Destroy();

    auto& cmdBuffers = vk::VkResources::Inst->modelCommandBuffers;
    for (std::size_t i = 0; i < vk::VkResources::Inst->GetViewportCount(); ++i)
        cmdBuffers[i].Destroy();
}
}    // namespace neko::vk