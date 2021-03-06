#pragma once
/*
 MIT License

 Copyright (c) 2020 SAE Institute Switzerland AG

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.

 Author : Guillaume Jeannin
 Co-Author : Floreau Luca
 Date : 13.10.2020
---------------------------------------------------------- */
#include "aer/editor/editor_tool_interface.h"

#include "utils/imgui_utility.h"

namespace neko::aer
{
class Inspector final : public EditorToolInterface
{
public:
	explicit Inspector(AerEngine& engine);

	void DrawImGui() override;

	[[nodiscard]] std::string_view GetName() const override { return "Inspector"; }
	[[nodiscard]] ToolType GetType() const override { return INSPECTOR; }

	// Not defined
	void Init() override {}
	void Update(seconds) override {}
	void Destroy() override {}
	void OnEvent(const SDL_Event&) override {}

private:
	void DisplayComponent(ComponentSerializer& serializer, Entity entity, ComponentType type);
	void DisplayLayersAndTags(Entity selectedEntity);
	void DisplayNewComponentButtons(Entity selectedEntity);

	// Component Managers
	EditorToolManager& editorToolManager_;
	EntityManager& entityManager_;
	Transform3dManager& transform3dManager_;
	RenderManager& renderManager_;
	LightManager& lightManager_;
	physics::RigidDynamicManager& rigidDynamicManager_;
	physics::RigidStaticManager& rigidStaticManager_;
#ifdef NEKO_FMOD
	AudioManager& audioManager_;
#endif

	// Component Viewers
	Transform3dSerializer& transform3dViewer_;
	RendererSerializer& rendererViewer_;
	LightSerializer& lightViewer_;
	physics::RigidDynamicSerializer& rigidDynamicViewer_;
	physics::RigidStaticSerializer& rigidStaticViewer_;
#ifdef NEKO_FMOD
	AudioSerializer& audioViewer_;
#endif

	// ImGui Variables
    std::string entityNameBuffer_;

	ImGui::ImGuiTextFilter searchFilter_;
	std::string newStr_;
	TagIndex currentTag_     = 0;
	LayerIndex currentLayer_ = 0;

	static constexpr std::array<ComponentType, 20> componentTypes_ {
		ComponentType::TRANSFORM2D,
		ComponentType::TRANSFORM3D,

		// Graphics
		ComponentType::SPRITE2D,
		ComponentType::MODEL,
		ComponentType::LIGHT,

		// Physics
		ComponentType::BODY2D,
		ComponentType::BOX_COLLIDER2D,
		ComponentType::CIRCLE_COLLIDER2D,
		ComponentType::POLYGON_COLLIDER2D,
		ComponentType::CONVEX_SHAPE2D,
		ComponentType::RIGID_STATIC,
		ComponentType::RIGID_DYNAMIC,

		// Miscellaneous
		ComponentType::PREFAB,
		ComponentType::AUDIO_SOURCE
	};
};
}    // namespace neko::aer
