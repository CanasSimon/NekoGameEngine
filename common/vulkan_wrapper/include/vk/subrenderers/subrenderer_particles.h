#pragma once
#include "vk/buffers/uniform_handle.h"
#include "vk/commands/command_buffer.h"
#include "vk/pipelines/pipeline_stage.h"
#include "vk/pipelines/render_pipeline.h"

namespace neko::vk
{
class SubrendererParticles final : public RenderPipeline
{
public:
	explicit SubrendererParticles(const PipelineStage& pipelineStage);
    void Destroy() const override;

	void Render(const CommandBuffer& commandBuffer) override;

	constexpr static int GetSubrendererIndex()
	{
		return static_cast<int>(SubrendererIndex::PARTICLES);
	}

private:
	UniformHandle uniformScene_;
};
}
