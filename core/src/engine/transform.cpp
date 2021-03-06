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
 */
#include "engine/transform.h"
#include "mathematics/transform.h"
#include "utils/imgui_utility.h"

#ifdef NEKO_PROFILE
#include "easy/profiler.h"
#endif

namespace neko
{
void Scale2dManager::AddComponent(Entity entity)
{
	ResizeIfNecessary(components_, entity, Vec2f::one);
	return ComponentManager::AddComponent(entity);
}

void Scale3dManager::AddComponent(Entity entity)
{
	ResizeIfNecessary(components_, entity, Vec3f::one);
	return ComponentManager::AddComponent(entity);
}

Transform2dManager::Transform2dManager(EntityManager& entityManager)
   : ComponentManager<Mat4f, EntityMask(neko::ComponentType::TRANSFORM2D)>(entityManager),
	 positionManager_(entityManager),
	 scaleManager_(entityManager),
	 rotationManager_(entityManager),
	 dirtyManager_(entityManager)
{
	entityManager_.get().RegisterOnChangeParent(this);
	dirtyManager_.RegisterComponentManager(this);
}

void Transform2dManager::Update()
{
#ifdef NEKO_PROFILE
	EASY_BLOCK("Update Transform");
#endif
	//dirtyManager_.UpdateDirtyEntities();
}

void Transform2dManager::UpdateDirtyComponent(Entity entity) { UpdateTransform(entity); }

Vec2f Transform2dManager::GetPosition(Entity entity) const
{
	return positionManager_.GetComponent(entity);
}

degree_t Transform2dManager::GetRotation(Entity entity) const
{
	return rotationManager_.GetComponent(entity);
}

Vec2f Transform2dManager::GetScale(Entity entity) const
{
	return scaleManager_.GetComponent(entity);
}

void Transform2dManager::SetPosition(Entity entity, Vec2f position)
{
	positionManager_.SetComponent(entity, position);
	dirtyManager_.SetDirty(entity);
}

void Transform2dManager::SetRotation(Entity entity, degree_t angles)
{
	rotationManager_.SetComponent(entity, angles);
	dirtyManager_.SetDirty(entity);
}

void Transform2dManager::SetScale(Entity entity, Vec2f scale)
{
	scaleManager_.SetComponent(entity, scale);
	dirtyManager_.SetDirty(entity);
}

void Transform2dManager::AddComponent(Entity entity)
{
	positionManager_.AddComponent(entity);
	scaleManager_.AddComponent(entity);
	scaleManager_.SetComponent(entity, Vec2f::one);
	rotationManager_.AddComponent(entity);
	return ComponentManager::AddComponent(entity);
}

void Transform2dManager::OnChangeParent(
	Entity entity, [[maybe_unused]] Entity newParent, [[maybe_unused]] Entity oldParent)
{
	//TODO change local transform to not change the global transform when changing parent
	dirtyManager_.SetDirty(entity);
}

void Transform2dManager::UpdateTransform(Entity entity)
{
	const auto eulerAngles =
		EulerAngles(degree_t(0), degree_t(0), -rotationManager_.GetComponent(entity));
	const auto scale    = scaleManager_.GetComponent(entity);
	const auto position = positionManager_.GetComponent(entity);

	Mat4f transform =
		Transform3d::Transform(Vec3f(position, 0.0f), eulerAngles, Vec3f(scale, 1.0f));

	const auto parent = entityManager_.get().GetParent(entity);
	if (parent != INVALID_ENTITY) { transform = GetComponent(parent) * transform; }

	SetComponent(entity, transform);
}

Transform3dManager::Transform3dManager(EntityManager& entityManager)
   : DoubleBufferComponentManager(entityManager),
	 position3DManager_(entityManager),
	 scale3DManager_(entityManager, Vec3f::one),
	 rotation3DManager_(entityManager),
	 dirtyManager_(entityManager)
{
	entityManager_.get().RegisterOnChangeParent(this);
	dirtyManager_.RegisterComponentManager(this);
}

void Transform3dManager::Init() { RendererLocator::get().RegisterSyncBuffersFunction(this); }

void Transform3dManager::Update()
{
#ifdef NEKO_PROFILE
	EASY_BLOCK("Update Transform");
#endif
	dirtyManager_.UpdateDirtyEntities();
}

void Transform3dManager::UpdateDirtyComponent(Entity entity) { UpdateTransform(entity); }

void Transform3dManager::UpdateTransform(Entity entity)
{
	Mat4f transform = Transform3d::Transform(position3DManager_.GetComponent(entity),
		rotation3DManager_.GetComponent(entity),
		scale3DManager_.GetComponent(entity));

	const auto parent = entityManager_.get().GetParent(entity);
	if (parent != INVALID_ENTITY) { transform = GetComponent(parent) * transform; }

	SetComponent(entity, transform);
}

Vec3f Transform3dManager::GetGlobalPosition(Entity entity) const
{
    return Transform3d::GetPosition(GetComponent(entity));
}

EulerAngles Transform3dManager::GetGlobalRotation(Entity entity) const
{
    return Transform3d::GetRotation(GetComponent(entity));
}

Vec3f Transform3dManager::GetGlobalScale(Entity entity) const
{
    return Transform3d::GetScale(GetComponent(entity));
}

void Transform3dManager::SetGlobalPosition(Entity entity, Vec3f position)
{
	Mat4f transform   = Transform3d::Transform(position,
        Transform3d::GetRotation(GetComponent(entity)),
        Transform3d::GetScale(GetComponent(entity)));
	const auto parent = entityManager_.get().GetParent(entity);
	if (parent != INVALID_ENTITY) transform = GetComponent(parent).Inverse() * transform;

	position3DManager_.SetComponent(entity, Transform3d::GetPosition(transform));
	UpdateTransform(entity);
	dirtyManager_.SetDirty(entity);
}

void Transform3dManager::SetGlobalRotation(Entity entity, EulerAngles angles)
{
    Mat4f transform   = Transform3d::Transform(Transform3d::GetPosition(GetComponent(entity)),
                                               angles,
                                               Transform3d::GetScale(GetComponent(entity)));
    const auto parent = entityManager_.get().GetParent(entity);
    if (parent != INVALID_ENTITY) { transform = GetComponent(parent).Inverse() * transform; }

    rotation3DManager_.SetComponent(entity, Transform3d::GetRotation(transform));
    UpdateTransform(entity);
    dirtyManager_.SetDirty(entity);
}

void Transform3dManager::SetGlobalScale(Entity entity, Vec3f scale)
{
    Mat4f transform = Transform3d::Transform(Transform3d::GetPosition(GetComponent(entity)),
                                             Transform3d::GetRotation(GetComponent(entity)), scale);
    const auto parent = entityManager_.get().GetParent(entity);
    if (parent != INVALID_ENTITY) { transform = GetComponent(parent).Inverse() * transform; }

    scale3DManager_.SetComponent(entity, Transform3d::GetScale(transform));
    UpdateTransform(entity);
    dirtyManager_.SetDirty(entity);
}

Vec3f Transform3dManager::GetPosition(Entity entity) const
{
    return position3DManager_.GetComponent(entity);
}

EulerAngles Transform3dManager::GetRotation(Entity entity) const
{
    return rotation3DManager_.GetComponent(entity);
}

Vec3f Transform3dManager::GetScale(Entity entity) const
{
    return scale3DManager_.GetComponent(entity);
}

void Transform3dManager::SetPosition(Entity entity, const Vec3f& position)
{
    position3DManager_.SetComponent(entity, position);
    UpdateTransform(entity);
    dirtyManager_.SetDirty(entity);
}

void Transform3dManager::SetRotation(Entity entity, const EulerAngles& angles)
{
    rotation3DManager_.SetComponent(entity, angles);
    UpdateTransform(entity);
    dirtyManager_.SetDirty(entity);
}

void Transform3dManager::SetScale(Entity entity, const Vec3f& scale)
{
	scale3DManager_.SetComponent(entity, scale);
	UpdateTransform(entity);
	dirtyManager_.SetDirty(entity);
}

void Transform3dManager::AddComponent(Entity entity)
{
	position3DManager_.AddComponent(entity);
	position3DManager_.SetComponent(entity, Vec3f::zero);
	scale3DManager_.AddComponent(entity);
	scale3DManager_.SetComponent(entity, Vec3f::one);
	rotation3DManager_.AddComponent(entity);
	rotation3DManager_.SetComponent(entity, EulerAngles::zero);
	return DoubleBufferComponentManager::AddComponent(entity);
}

void Transform3dManager::OnChangeParent(Entity entity, Entity, Entity)
{
	Mat4f transform = GetComponent(entity);
	SetScale(entity, Transform3d::GetScale(transform));
	SetRotation(entity, Transform3d::GetRotation(transform));
	SetPosition(entity, Transform3d::GetPosition(transform));
	dirtyManager_.SetDirty(entity);
}

Transform3dSerializer::Transform3dSerializer(
	EntityManager& entityManager, Transform3dManager& transform3dManager)
   : ComponentSerializer(entityManager), transform3dManager_(transform3dManager)
{}

json Transform3dSerializer::GetJsonFromComponent(Entity entity) const
{
	json transformComponent = json::object();
	if (entityManager_.HasComponent(entity, EntityMask(ComponentType::TRANSFORM3D)))
	{
		if (entity != INVALID_ENTITY && entityManager_.GetEntitiesSize() > entity)
		{
			transformComponent["position"] =
				GetJsonFromVector3(transform3dManager_.GetPosition(entity));
			transformComponent["rotation"] = GetJsonFromVector4(
				Vec4f(Quaternion::FromEuler(transform3dManager_.GetRotation(entity))));
			transformComponent["scale"] = GetJsonFromVector3(transform3dManager_.GetScale(entity));
		}
	}

	return transformComponent;
}

void Transform3dSerializer::SetComponentFromJson(Entity entity, const json& jsonComponent)
{
	if (CheckJsonParameter(jsonComponent, "position", json::object()))
	{
		const Vec3f position = GetVector3FromJson(jsonComponent, "position");
		transform3dManager_.SetPosition(entity, position);
	}

	if (CheckJsonParameter(jsonComponent, "rotation", json::object()))
	{
		const Quaternion rotation = Quaternion(GetVector4FromJson(jsonComponent, "rotation"));
		const EulerAngles angles  = Quaternion::ToEulerAngles(rotation);
		transform3dManager_.SetRotation(entity, angles);
	}

	if (CheckJsonParameter(jsonComponent, "scale", json::object()))
	{
		transform3dManager_.SetScale(entity, GetVector3FromJson(jsonComponent, "scale"));
	}
}

void Transform3dSerializer::DrawImGui(Entity entity)
{
	if (entity == INVALID_ENTITY) return;
	if (entityManager_.HasComponent(entity, EntityMask(ComponentType::TRANSFORM3D)))
	{
		if (ImGui::TreeNode("Transform"))
		{
			static bool globalPos;
			ImGui::Checkbox("GlobalPos", &globalPos);

			Vec3f position;
			if (globalPos) position = transform3dManager_.GetGlobalPosition(entity);
			else position = transform3dManager_.GetPosition(entity);
			if (ImGui::DragFloat3("Position", position.coord, ImGui::LabelPos::LEFT, 0.05f))
				transform3dManager_.SetPosition(entity, position);

			Vec3f rotation;
			if (globalPos) rotation = Vec3f(transform3dManager_.GetGlobalRotation(entity));
			else rotation = Vec3f(transform3dManager_.GetRotation(entity));
			if (ImGui::DragFloat3("Rotation", rotation.coord, ImGui::LabelPos::LEFT, 0.05f))
				transform3dManager_.SetRotation(
					entity, EulerAngles(rotation.x, rotation.y, rotation.z));

			Vec3f scale;
			if (globalPos) scale = transform3dManager_.GetGlobalScale(entity);
			else scale = transform3dManager_.GetScale(entity);
			if (ImGui::DragFloat3("Scale", scale.coord, ImGui::LabelPos::LEFT, 0.05f))
				transform3dManager_.SetScale(entity, scale);
			ImGui::TreePop();
		}
	}
}
}
