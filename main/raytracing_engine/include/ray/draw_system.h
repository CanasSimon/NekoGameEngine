#pragma once
#include "engine/transform.h"
#include "graphics/lights.h"

#include "sdl_engine/sdl_camera.h"

#include "vk/material/material_manager.h"
#include "vk/models/model_manager.h"

namespace neko
{
constexpr ImGuiDockNodeFlags kDockspaceFlags = ImGuiDockNodeFlags_NoDockingInCentralNode |
                                               ImGuiDockNodeFlags_AutoHideTabBar |
                                               ImGuiDockNodeFlags_PassthruCentralNode;
class DrawSystem final : public SystemInterface,
						 public DrawImGuiInterface,
						 public sdl::SdlEventSystemInterface,
						 public RenderCommandInterface
{
public:
	void Init() override;
	void Update(seconds dt) override;
	void Render() override;
	void Destroy() override;

	void OnEvent(const SDL_Event& event) override;
	void DrawImGui() override;

private:
	enum InstanceIds : std::uint8_t
    {
		PLANE = 0,
		CUBE
	};

	seconds dt_ {};
	float timeSinceUpdate_;
	float fpsCache_;

	sdl::MultiCamera camera_;

	vk::TextureManager textureManager_;
	vk::MaterialManager materialManager_;
	vk::ModelManager modelManager_;

    DirectionalLight dirLight_ {};

	vk::ModelId planeModelId_ = sole::uuid();
	vk::ModelId cubeModelId_ = sole::uuid();
};
}    // namespace neko
