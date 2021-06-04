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
 */
#include <unordered_map>
#include <vector>

#include "engine/globals.h"
#include "engine/system.h"
#include "mathematics/hash.h"
#include "utils/action_utility.h"

namespace neko
{
class OnChangeParentInterface;

using Entity     = Index;
using EntityMask = std::uint32_t;

constexpr Entity INVALID_ENTITY          = std::numeric_limits<Index>::max();
constexpr EntityMask INVALID_ENTITY_MASK = 0u;
enum class ComponentType : std::uint32_t;

template<typename T, EntityMask componentType>
class ComponentManager;

/// Used in an Entity-Component-System to store all entities and what components they have
class EntityManager
{
public:
	explicit EntityManager();

	EntityMask GetMask(Entity entity);

    template<typename T, EntityMask componentType>
    void RegisterComponentManager(ComponentManager<T, componentType>& componentManager)
    {
        onDestroyEntity.RegisterCallback(
            [&componentManager](Entity entity) { componentManager.DestroyComponent(entity); });
    }

    void RegisterOnChangeParent(OnChangeParentInterface* onChangeInterface);

    /// Create an empty entity (non-null EntityMask)
	Entity CreateEntity(Entity entity = INVALID_ENTITY);
	Entity GetLastEntity();
    void DestroyEntity(Entity entity, bool destroyChildren = true);

	void Clear();

    void AddComponentType(Entity entity, EntityMask componentType);
    void RemoveComponentType(Entity entity, EntityMask componentType);

	[[nodiscard]] size_t GetEntitiesNmb(EntityMask filterComponents = INVALID_ENTITY_MASK);
	[[nodiscard]] size_t GetEntitiesSize() const;
	[[nodiscard]] std::vector<Entity> FilterEntities(
		EntityMask filterComponents = INVALID_ENTITY_MASK) const;

    [[nodiscard]] std::string_view GetEntityName(Entity entity) const;

    [[nodiscard]] Entity GetParent(Entity entity) const;

    /// Return the first root existing entity or INVALID_ENTITY
    [[nodiscard]] Entity GetFirstRoot() const;

	void SetEntityName(Entity entity, std::string_view entityName);

    /// Set the entity parent and check if child is not recursive parent
    bool SetParent(Entity child, Entity parent);

    [[nodiscard]] bool HasComponent(Entity entity, EntityMask componentType) const;
    [[nodiscard]] bool IsPrefab(Entity entity) const;
    [[nodiscard]] bool EntityExists(Entity entity) const;

private:
	Action<Entity> onDestroyEntity;
	Action<Entity, Entity, Entity> onChangeParent;
	std::vector<Entity> parentEntities_;
	std::vector<EntityMask> entityMasks_;
	std::vector<std::string> entityNames_;
};

class DirtyManager
{
public:
	explicit DirtyManager(EntityManager& entityManager);

	void UpdateDirtyEntities();

	template<typename T, EntityMask componentType>
	void RegisterComponentManager(ComponentManager<T, componentType>* componentManager)
	{
		updateDirtyEntity.RegisterCallback(
			[componentManager](Entity entity) { componentManager->UpdateDirtyComponent(entity); });
	}

	void SetDirty(Entity entity);

private:
	std::reference_wrapper<EntityManager> entityManager_;
	Action<Entity> updateDirtyEntity;
	std::vector<Entity> dirtyEntities_;
};

class OnChangeParentInterface
{
public:
	virtual ~OnChangeParentInterface()                                             = default;
	virtual void OnChangeParent(Entity entity, Entity newParent, Entity oldParent) = 0;
};
}    // namespace neko
