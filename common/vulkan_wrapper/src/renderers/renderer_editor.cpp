#include "vk/renderers/renderer_editor.h"

#include "vk/subrenderers/subrenderer_opaque.h"
#include "vk/subrenderers/subrenderer_particles.h"

namespace neko::vk
{
void RendererEditor::Init()
{
	std::vector<Attachment> renderPassAttachments = {
		{0, "color", Attachment::Type::SWAPCHAIN, false},
		{1, "depth", Attachment::Type::DEPTH, false},
	};

	std::vector<SubpassType> renderPassSubpasses = {
		{0, {0, 1}},    //Geometry pass
		{1, {0, 1}}     //Post process pass
	};

	SetRenderStage(std::make_unique<RenderStage>(renderPassAttachments, renderPassSubpasses));
}

void RendererEditor::Start()
{
	started_ = true;

	AddSubrenderer<SubrendererOpaque>({0, 0});
	AddSubrenderer<SubrendererParticles>({0, 1});
}

void RendererEditor::Destroy() const
{
	rendererContainer_.Destroy();
	renderStage_->Destroy();
}
}    // namespace neko::vk
