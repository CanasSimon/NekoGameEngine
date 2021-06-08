#include "vk/subrenderers/subrenderer_opaque.h"

#include "sdl_engine/sdl_camera.h"

#include "vk/material/material_manager.h"
#include "vk/vk_resources.h"

namespace neko::vk
{
SubrendererOpaque::SubrendererOpaque(PipelineStage stage) : RenderPipeline(stage) {}

void SubrendererOpaque::Destroy() const
{
	for (auto& uniformScene : uniformScenes_) uniformScene.Destroy();
	for (auto& uniformLight : uniformLights_) uniformLight.Destroy();

	auto& cmdBuffers = vk::VkResources::Inst->modelCommandBuffers;
	for (std::size_t i = 0; i < vk::VkResources::Inst->GetViewportCount(); ++i)
		cmdBuffers[i].Destroy();
}

void SubrendererOpaque::Render(const CommandBuffer& commandBuffer)
{
	const auto& cameras                = sdl::MultiCameraLocator::get();
	VkResources* vkObj                 = VkResources::Inst;
	const RenderStage& renderStage     = vkObj->GetRenderStage();
	const std::uint8_t viewportCount   = vkObj->GetViewportCount();
	const LightCommandBuffer& lightCmd = vkObj->lightCommandBuffer;

	VkRect2D renderArea;
	renderArea.offset = {0, 0};
	renderArea.extent = {static_cast<uint32_t>(renderStage.GetSize().x),
		static_cast<uint32_t>(renderStage.GetSize().y)};

	for (std::size_t i = 0; i < viewportCount; ++i)
	{
		ModelCommandBuffer& cmdBuffer = vkObj->modelCommandBuffers[i];
		cmdBuffer.PrepareData();

		ChooseViewport(commandBuffer, renderArea, viewportCount, i);

		const Mat4f view = cameras.GenerateViewMatrix(i);
		Mat4f proj       = cameras.GenerateProjectionMatrix(i);
		proj[1][1] *= -1.0f;

		uniformScenes_[i].Push(kProjHash, proj);
		uniformScenes_[i].Push(kViewHash, view);
		uniformScenes_[i].Push(kViewPosHash, cameras.GetPosition(i));

		const std::size_t lightNum = lightCmd.GetLightNum();
		uniformLights_[i].Push(kLightNumHash, &lightNum);
		uniformLights_[i].Push(kDirLightHash, *DirectionalLight::Instance);

		//GPU Instancing
		for (auto&& meshInstance : cmdBuffer.GetModelInstances())
			meshInstance.CmdRender(commandBuffer, uniformScenes_[i], uniformLights_[i]);
	}
}

void SubrendererOpaque::ChooseViewport(const CommandBuffer& cmdBuffer,
	const VkRect2D& renderArea,
	std::uint8_t viewportCount,
	std::uint8_t viewportIndex)
{
	switch (viewportCount)
	{
		case 1:
		{
			VkViewport viewport;
			viewport.x        = 0.0f;
			viewport.y        = 0.0f;
			viewport.width    = static_cast<float>(renderArea.extent.width);
			viewport.height   = static_cast<float>(renderArea.extent.height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
			break;
		}
		case 2:
		{
			const auto width = static_cast<float>(renderArea.extent.width) / 2.0f;
			const auto height = static_cast<float>(renderArea.extent.height);
			const auto indexX = static_cast<float>(viewportIndex);

			VkViewport viewport;
			viewport.x        = width * indexX;
			viewport.y        = 0.0f;
			viewport.width    = width;
			viewport.height   = height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
			break;
		}
		case 3:
		case 4:
		{
			const auto width = static_cast<float>(renderArea.extent.width) / 2.0f;
			const auto height = static_cast<float>(renderArea.extent.height) / 2.0f;
			const auto indexX = static_cast<float>(viewportIndex % 2);
			const auto indexY = static_cast<float>(viewportIndex / 2 % 2);

			VkViewport viewport;
			viewport.x        = width * indexX;
			viewport.y        = height * indexY;
			viewport.width    = width;
			viewport.height   = height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
			break;
		}
		case 0:
		default: neko_assert(false, "Invalid Viewport number!!");
	}
}
}    // namespace neko::vk