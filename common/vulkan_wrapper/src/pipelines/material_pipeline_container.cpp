#include "vk/pipelines/material_pipeline_container.h"

namespace neko::vk
{
MaterialPipeline& MaterialPipelineContainer::AddMaterial(
	const PipelineStage& pipelineStage, const GraphicsPipelineCreateInfo& pipelineCreate)
{
	for (std::size_t i = 0; i < registeredInfos_.size(); i++)
		if (registeredInfos_[i].createInfo == pipelineCreate) return registeredMaterials_[i];

	registeredInfos_.push_back(PipelineInfo {pipelineStage, pipelineCreate});
	return registeredMaterials_.emplace_back(pipelineStage, pipelineCreate);
}

std::optional_ref<MaterialPipeline> MaterialPipelineContainer::GetMaterial(
	const PipelineStage& pipelineStage, const GraphicsPipelineCreateInfo& pipelineCreate)
{
	auto i = 0;
	for (const auto& infoMaterial : registeredInfos_)
	{
		if (infoMaterial.stage == pipelineStage && infoMaterial.createInfo == pipelineCreate)
			return registeredMaterials_[i];
		i++;
	}

	LogDebug("Can't access material!");
	return std::nullopt;
}
}    // namespace neko::vk