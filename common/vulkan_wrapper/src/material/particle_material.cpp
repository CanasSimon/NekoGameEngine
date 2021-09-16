#include "vk/material/particle_material.h"

#include "vk/particle/particle_instance.h"
#include "vk/pipelines/material_pipeline.h"

namespace neko::vk
{
ParticleMaterial::ParticleMaterial(const std::optional_const_ref<Image2d>& diffuse)
   : diffuse_(diffuse)
{
	renderMode_ = RenderMode::VK_TRANSPARENT;

    if (diffuse_)
	{
        const auto diffuse = MaterialExportData(&diffuse_->get());
        descriptorData_.emplace(kDiffuseHash, diffuse);
    }
}

bool ParticleMaterial::operator==(const ParticleMaterial& other) const
{
	return diffuse_ && other.diffuse_ && &diffuse_.value() == &other.diffuse_.value() ||
	       !diffuse_ && !other.diffuse_;
}

bool ParticleMaterial::operator!=(const ParticleMaterial& other) const
{
    return !(*this == other);
}

std::string ParticleMaterial::GetShaderPath() const
{
	return GetVkShadersFolderPath() + "particles/particle.aershader";
}

void ParticleMaterial::CreatePipeline(bool forceRecreate)
{
	if (pipelineMaterial_.IsBuilt() && !forceRecreate) return;

	ResetPipeline();
}

void ParticleMaterial::SetDiffuse(const Image2d& diffuseTex)
{
	diffuse_ = std::optional_const_ref<Image2d>(diffuseTex);

	auto diffuse = MaterialExportData(&diffuse_->get());
	for (auto& data : descriptorData_)
	{
		if (data.first == kDiffuseHash)
		{
			data.second = diffuse;
			return;
		}
	}

	descriptorData_.emplace(kDiffuseHash, diffuse);
	ResetPipeline();
}

void ParticleMaterial::ResetDiffuse()
{
	diffuse_.reset();

	for (auto it = descriptorData_.begin(); it != descriptorData_.end(); ++it)
	{
		if (it->first == kDiffuseHash)
		{
			descriptorData_.erase(it);
			break;
		}
	}

	ResetPipeline();
}

void ParticleMaterial::ResetPipeline()
{
	const PipelineStage stage =
		renderMode_ == RenderMode::VK_OPAQUE ? PipelineStage {0, 0} : PipelineStage {0, 1};

	const auto createInfo = GraphicsPipelineCreateInfo(GetShaderPath(),
		{Vertex::GetVertexInput(0), ParticleInstance::Instance::GetVertexInput(1)},
		GraphicsPipeline::Mode::MRT,
		GraphicsPipeline::Depth::READ,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		VK_POLYGON_MODE_FILL,
		VK_CULL_MODE_NONE);

	pipelineMaterial_ = MaterialPipeline::Create(stage, createInfo);
}
}