#pragma once
#include <optional>

#include "vk/images/image2d.h"
#include "vk/material/material.h"

namespace neko::vk
{
class ParticleMaterial : public Material
{
public:
	explicit ParticleMaterial(
		const std::optional_const_ref<Image2d>& diffuse = std::nullopt);

	bool operator==(const ParticleMaterial& other) const;
    bool operator!=(const ParticleMaterial& other) const;

	[[nodiscard]] std::string GetShaderPath() const override;
	[[nodiscard]] MaterialType GetType() const override { return MaterialType::PARTICLE; }

	void CreatePipeline(bool forceRecreate) override;

	[[nodiscard]] std::optional_const_ref<Image2d> GetDiffuse() const { return diffuse_; }
	void SetDiffuse(const Image2d& diffuseTex);
	void ResetDiffuse();

	[[nodiscard]] RenderMode GetRenderMode() const override { return renderMode_; }
	void SetRenderMode(const RenderMode renderMode) override { renderMode_ = renderMode; }

	[[nodiscard]] ordered_json ToJson() const override { return ordered_json(); }
	void FromJson(const json& materialJson) override {}

private:
    void ResetPipeline();

    std::optional_const_ref<Image2d> diffuse_;
};
}
