
#include <vk/pipelines/material_pipeline.h>

#include "vk/vk_resources.h"

namespace neko::vk
{
MaterialPipeline::MaterialPipeline(
	PipelineStage pipelineStage, GraphicsPipelineCreateInfo pipelineCreate)
   : pipelineStage_(pipelineStage),
	 pipelineGraphicsCreate_(std::move(pipelineCreate)),
	 renderStage_(std::nullopt)
{}

void MaterialPipeline::Destroy()
{
	pipeline_.Destroy();
	isBuilt_ = false;
}

MaterialPipeline& MaterialPipeline::Create(
	const PipelineStage& pipelineStage, const GraphicsPipelineCreateInfo& pipelineCreate)
{
	return VkResources::Inst->AddMaterialPipeline(pipelineStage, pipelineCreate);
}

bool MaterialPipeline::BindPipeline(const CommandBuffer& commandBuffer)
{
	const RenderStage& renderStage = VkResources::Inst->GetRenderStage();
	if (!renderStage_ || &renderStage_->get() != &renderStage)
	{
		renderStage_.emplace(renderStage);

		pipeline_ = GraphicsPipeline(pipelineStage_, pipelineGraphicsCreate_);
        isBuilt_ = true;
	}

	pipeline_.BindPipeline(commandBuffer);
	return true;
}
}    // namespace neko::vk