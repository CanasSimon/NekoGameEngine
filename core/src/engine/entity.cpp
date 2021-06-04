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
#include <engine/component.h>
#include <engine/entity.h>
#include <engine/log.h>
#include <utils/vector_utility.h>
#include <algorithm>
#include <sstream>
#include "engine/globals.h"

#include <fmt/format.h>
#include "imgui.h"

namespace neko
{
//-----------------------------------------------------------------------------
// EntityManager
//-----------------------------------------------------------------------------
EntityManager::EntityManager()
{
	entityMasks_.resize(INIT_ENTITY_NMB);
	entityNames_.resize(INIT_ENTITY_NMB);
	parentEntities_.resize(INIT_ENTITY_NMB, INVALID_ENTITY);
}

EntityMask EntityManager::GetMask(Entity entity) { return entityMasks_[entity]; }

void EntityManager::RegisterOnChangeParent(OnChangeParentInterface* onChangeInterface)
{
	onChangeParent.RegisterCallback(
		[onChangeInterface](Entity entity, Entity newParent, Entity oldParent)
		{ onChangeInterface->OnChangeParent(entity, newParent, oldParent); });
}

Entity EntityManager::CreateEntity(Entity entity)
{
	if (entity == INVALID_ENTITY)
	{
		const auto entityMaskIt = std::find_if(entityMasks_.begin(),
			entityMasks_.end(),
			[](EntityMask entityMask) { return entityMask == INVALID_ENTITY_MASK; });

		EntityMask newEntity;
		if (entityMaskIt == entityMasks_.end())
		{
			newEntity = entityMasks_.size();
			ResizeIfNecessary(entityMasks_, newEntity, INVALID_ENTITY_MASK);
			ResizeIfNecessary(parentEntities_, newEntity, INVALID_ENTITY);
			ResizeIfNecessary(entityNames_, newEntity, std::string());
		}
		else
		{
			newEntity = entityMaskIt - entityMasks_.begin();
		}

		const std::string name  = "Entity " + std::to_string(newEntity);
		entityNames_[newEntity] = name;

		AddComponentType(Entity(newEntity), static_cast<EntityMask>(ComponentType::EMPTY));
		return Entity(newEntity);
	}
	else
	{
		ResizeIfNecessary(entityMasks_, entity, INVALID_ENTITY_MASK);
		ResizeIfNecessary(parentEntities_, entity, INVALID_ENTITY);
		ResizeIfNecessary(entityNames_, entity, std::string());
		if (!EntityExists(entity))
		{
			AddComponentType(entity, static_cast<EntityMask>(ComponentType::EMPTY));
			return entity;
		}

        const std::string name = "Entity " + std::to_string(entity);
		entityNames_[entity] = name;

		return CreateEntity(INVALID_ENTITY);
	}
}

Entity EntityManager::GetLastEntity()
{
	const auto it = std::find_if(entityMasks_.rbegin(),
		entityMasks_.rend(),
		[](EntityMask entityMask) { return entityMask != INVALID_ENTITY_MASK; });

	return Entity(std::distance(entityMasks_.begin(), it.base()) - 1);
}

void EntityManager::DestroyEntity(Entity entity, bool destroyChildren)
{
	entityMasks_[entity] = INVALID_ENTITY_MASK;
	entityNames_[entity] = "";
	if (destroyChildren)
	{
		for (Entity childEntity = 0; childEntity < parentEntities_.size(); ++childEntity)
			if (parentEntities_[childEntity] == entity) DestroyEntity(childEntity, true);
	}
	else
	{
		for (auto&& parentEntity : parentEntities_)
			if (parentEntity == entity) parentEntity = parentEntities_[entity];
	}

	parentEntities_[entity] = INVALID_ENTITY;
	onDestroyEntity.Execute(entity);
}

void EntityManager::Clear()
{
	for (Entity entity = 0; entity < GetEntitiesSize(); ++entity) DestroyEntity(entity, false);
}

void EntityManager::AddComponentType(Entity entity, EntityMask componentType)
{
	entityMasks_[entity] |= static_cast<EntityMask>(componentType);
}

void EntityManager::RemoveComponentType(Entity entity, EntityMask componentType)
{
	entityMasks_[entity] &= ~static_cast<EntityMask>(componentType);
}

size_t EntityManager::GetEntitiesNmb(EntityMask filterComponents)
{
	return std::count_if(entityMasks_.begin(),
		entityMasks_.end(),
		[&filterComponents](EntityMask entityMask)
		{
			return entityMask != INVALID_ENTITY_MASK &&
		           (entityMask & static_cast<EntityMask>(filterComponents)) ==
		               static_cast<EntityMask>(filterComponents);
		});
}

size_t EntityManager::GetEntitiesSize() const { return entityMasks_.size(); }

std::vector<Entity> EntityManager::FilterEntities(EntityMask filterComponents) const
{
	std::vector<Entity> entities;
	entities.reserve(entityMasks_.size());
	for (Entity i = 0; i < entityMasks_.size(); i++)
	{
		if (HasComponent(i, filterComponents)) entities.push_back(i);
	}
	return entities;
}

std::string_view EntityManager::GetEntityName(Entity entity) const { return entityNames_[entity]; }

Entity EntityManager::GetParent(Entity entity) const { return parentEntities_[entity]; }

Entity EntityManager::GetFirstRoot() const
{
	for (Entity entity = 0; entity < GetEntitiesSize(); entity++)
	{
		if (EntityExists(entity) && GetParent(entity) == INVALID_ENTITY) return entity;
	}

	return INVALID_ENTITY;
}

void EntityManager::SetEntityName(Entity entity, std::string_view entityName)
{
	entityNames_[entity] = entityName.data();
}

bool EntityManager::SetParent(Entity child, Entity parent)
{
	const auto oldParent = GetParent(child);
	auto p               = GetParent(parent);
	while (p != INVALID_ENTITY)
	{
		if (p == child)
		{
			logDebug(fmt::format(
				"[Warning] Child entity: {} cannot have parent entity: {}", child, parent));
			return false;
		}

		p = GetParent(p);
	}

	parentEntities_[child] = parent;
	onChangeParent.Execute(child, parent, oldParent);
	return true;
}

bool EntityManager::HasComponent(Entity entity, EntityMask componentType) const
{
	if (entity >= entityMasks_.size())
	{
		std::ostringstream oss;
		oss << "[Error] Accessing entity: " << entity
			<< " while entity mask array is of size: " << entityMasks_.size();
		logDebug(oss.str());
		return false;
	}
	return (entityMasks_[entity] & static_cast<EntityMask>(componentType)) ==
	       static_cast<EntityMask>(componentType);
}

bool EntityManager::IsPrefab(Entity entity) const
{
	return HasComponent(entity, static_cast<EntityMask>(ComponentType::PREFAB));
}

bool EntityManager::EntityExists(Entity entity) const
{
	return entityMasks_[entity] != INVALID_ENTITY_MASK;
}

//-----------------------------------------------------------------------------
// DirtyManager
//-----------------------------------------------------------------------------
DirtyManager::DirtyManager(EntityManager& entityManager) : entityManager_(entityManager) {}

void DirtyManager::UpdateDirtyEntities()
{
	// Fill the dirty entities with all the children in O(n)
	for (Entity entity = 0; entity < entityManager_.get().GetEntitiesSize(); entity++)
	{
		if (!entityManager_.get().EntityExists(entity)) continue;
		auto parent = entityManager_.get().GetParent(entity);
		while (parent != INVALID_ENTITY)
		{
			if (std::find(dirtyEntities_.cbegin(), dirtyEntities_.cend(), parent) !=
				dirtyEntities_.end())
			{
				SetDirty(entity);
				break;
			}
			parent = entityManager_.get().GetParent(parent);
		}
	}

	for (auto entity : dirtyEntities_) updateDirtyEntity.Execute(entity);
	dirtyEntities_.clear();
}

void DirtyManager::SetDirty(Entity entity)
{
	if (std::find(dirtyEntities_.cbegin(), dirtyEntities_.cend(), entity) == dirtyEntities_.cend())
	{
		dirtyEntities_.push_back(entity);
	}
}
}    // namespace neko
