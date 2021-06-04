#pragma once
/* ----------------------------------------------------
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

 Author : Floreau Luca
 Co-Author :
 Date : 22.11.2020
---------------------------------------------------------- */
#include "aer/managers/render_manager.h"

#include "px/collider.h"
#include "px/filter_group.h"
#include "px/physics_callbacks.h"

#ifdef NEKO_GLES3
#include "gl/model_manager.h"
#else
#include "vk/models/model_manager.h"
#endif

namespace neko::physics
{
class PhysicsEngine;

//-----------------------------------------------------------------------------
// Actors
//-----------------------------------------------------------------------------
/// All data used by RigidActor
struct RigidActorData
{
	PhysicsMaterial material;
	ColliderType colliderType = ColliderType::INVALID;
	BoxColliderData boxColliderData;
	SphereColliderData sphereColliderData;
	CapsuleColliderData capsuleColliderData;
	MeshColliderData meshColliderData;
	FilterGroup::Enum filterGroup = FilterGroup::DEFAULT;
};

/// Struct containing pointers of PxMaterial and PxShape and functions to read physics info
class RigidActor
{
public:
	[[nodiscard]] physx::PxShape* GetPxShape() const { return shape_; }
	[[nodiscard]] physx::PxMaterial* GetPxMaterial() const { return material_; }

	[[nodiscard]] ColliderType GetColliderType() const;

    /// Get the data of an actor <br>
    /// Must be called in FixedUpdate or when physics is not running
    [[nodiscard]] PhysicsMaterial GetPhysicsMaterial() const;

    /// Get the data of an actor <br>
    /// Must be called in FixedUpdate or when physics is not running
    [[nodiscard]] BoxColliderData GetBoxColliderData() const;

	/// Get the data of an actor <br>
	/// Must be called in FixedUpdate or when physics is not running
	[[nodiscard]] SphereColliderData GetSphereColliderData() const;

	/// Get the data of an actor <br>
	/// Must be called in FixedUpdate or when physics is not running
	[[nodiscard]] CapsuleColliderData GetCapsuleColliderData() const;

    /// Set the material of an actor <br>
    /// Must be called in FixedUpdate or when physics is not running
    void SetMaterial(const PhysicsMaterial& physicsMaterial) const;

    /// Set the data of an actor <br>
    /// Must be called in FixedUpdate or when physics is not running
    void SetBoxColliderData(const BoxColliderData& boxColliderData) const;

	/// Get the data of an actor <br>
	/// Must be called in FixedUpdate or when physics is not running
	void SetSphereColliderData(const SphereColliderData& sphereColliderData) const;

	/// Set the data of an actor <br>
	/// Must be called in FixedUpdate or when physics is not running
	void SetCapsuleColliderData(const CapsuleColliderData& capsuleColliderData) const;

	/// Set the data of an actor <br>
	/// Must be called in FixedUpdate or when physics is not running
	void SetMeshColliderData(const MeshColliderData& meshColliderData) const;

protected:
	static physx::PxMaterial* InitMaterial(
		physx::PxPhysics* physics, const PhysicsMaterial& material);

	static physx::PxShape* InitBoxShape(
		physx::PxPhysics* physics, physx::PxMaterial* material, const BoxColliderData& boxCollider);
    static physx::PxShape* InitSphereShape(physx::PxPhysics* physics,
		physx::PxMaterial* material,
		const SphereColliderData& sphereCollider);
	static physx::PxShape* InitCapsuleShape(physx::PxPhysics* physics,
		physx::PxMaterial* material,
		const CapsuleColliderData& capsuleCollider);
	static physx::PxShape* InitMeshCollider(const PhysicsEngine& physics,
		physx::PxMaterial* material,
#ifdef NEKO_GLES3
		const gl::Mesh& mesh,
#else
		const vk::Mesh& mesh,
#endif
		const physx::PxMeshScale& scale);

	static void SetFiltering(physx::PxShape* shape, physx::PxU32 filterGroup);

	physx::PxMaterial* material_ = nullptr;
	physx::PxShape* shape_       = nullptr;
};

/// Used to serialize colliders and materials to / from json and imgui
class RigidActorSerializer
{
protected:
    /// Get the json object of the material of a RigidActorData
    /// \return json object with the material parameter
    [[nodiscard]] static json GetJsonFromMaterial(const RigidActorData& rigidActorData);

	/// Get the json object of the boxCollider of a RigidActorData
	/// \return json object with the boxCollider parameter
	[[nodiscard]] static json GetJsonFromBoxCollider(const RigidActorData& rigidActorData);

	/// Get the json object of the sphereCollider of a RigidActorData
	/// \return json object with the sphereCollider parameter
	[[nodiscard]] static json GetJsonFromSphereCollider(const RigidActorData& rigidActorData);

	/// Get the json object of the sphereCollider of a RigidActorData
	/// \return json object with the sphereCollider parameter
	[[nodiscard]] static json GetJsonFromCapsuleCollider(const RigidActorData& rigidActorData);

	/// Create a RigidActor from a json of a rigidActor
	static RigidActorData SetFromJson(const json& rigidActorJson);

	/// Used in the inspector to display different variables
	static RigidActorData DrawActorImGui(const RigidActorData& rigidActorData);
};

//-----------------------------------------------------------------------------
// Static RigidBodies
//-----------------------------------------------------------------------------
/// Data used by Static RigidBodies
struct RigidStaticData : public RigidActorData
{
	RigidStaticData()  = default;
	~RigidStaticData() = default;
};

/// Wrapper around PhysX values
class RigidStatic : public RigidActor
{
public:
	/// Init a rigidStatic actor in physx scene
	/// \param physics the PxPhysics pointer of the physicsEngine
	/// \param rigidStatic the data used to generate the rigidStatic
	/// \param position initial position of the actor
	/// \param eulerAngle initial rotation of the actor
	void Init(const PhysicsEngine& physics,
		const RigidStaticData& rigidStatic,
		const Vec3f& position,
		const EulerAngles& eulerAngle);

	[[nodiscard]] physx::PxRigidStatic* GetPxRigidStatic() const { return rigidActor_; }

    /// Get the data of an actor <br>
    /// Must be called in FixedUpdate or when physics is not running
    [[nodiscard]] RigidStaticData GetRigidStaticData() const;

	/// Set the data of an actor <br>
	/// Must be called in FixedUpdate or when physics is not running
	void SetRigidStaticData(const RigidStaticData& rigidStaticData) const;

private:
	physx::PxRigidStatic* rigidActor_ = nullptr;
};

/// Manager for Static RigidBodies
class RigidStaticManager final
   : public ComponentManager<RigidStatic, EntityMask(ComponentType::RIGID_STATIC)>,
	 public FixedUpdateInterface
{
public:
	RigidStaticManager(EntityManager& entityManager,
		Transform3dManager& transform3dManager,
		aer::RenderManager& renderManager,
		PhysicsEngine& physicsEngine);

	void FixedUpdate(seconds dt) override;

    void DestroyComponent(Entity entity) override;

    /// Find the Entity from an actor
    Entity FindEntityFromActor(physx::PxActor* actor);

	/// Create a RigidStatic Actor with the given RigidStaticData
	void AddRigidStatic(Entity entity, const RigidStaticData& body);

	/// Create a static MeshCollider from the given model
	void AddMeshColliderStatic(Entity entity, std::string_view modelName);

	/// Get the data of an actor <br>
	/// Must be called in FixedUpdate or when physics is not running
	[[nodiscard]] const RigidStaticData& GetRigidStaticData(Entity entity) const;

	/// Set the data of an actor <br>
	/// Must be called in FixedUpdate or when physics is not running
	void SetRigidStaticData(Entity entity, const RigidStaticData& rigidStaticData) const;

protected:
	Transform3dManager& transform3dManager_;
	PhysicsEngine& physicsEngine_;
	aer::RenderManager& renderManager_;

#ifdef NEKO_GLES3
	std::map<Entity, gl::ModelId> meshColliderToCreate_;
#else
	std::map<Entity, vk::ModelId> meshColliderToCreate_;
#endif
};

/// Used to serialize colliders and materials to / from json and imgui
class RigidStaticSerializer final : public ComponentSerializer,
									public FixedUpdateInterface,
									public RigidActorSerializer
{
public:
	RigidStaticSerializer(Transform3dManager& transform3dManager,
		EntityManager& entityManager,
		PhysicsEngine& physicsEngine,
		RigidStaticManager& rigidStaticManager);

	void FixedUpdate(seconds dt) override {}

    void SetSelectedEntity(Entity selectedEntity);

	/// Get the json object from a RigidStatic component
	[[nodiscard]] json GetJsonFromComponent(Entity entity) const override;

	/// Set a RigidStatic component from a json object
	void SetComponentFromJson(Entity entity, const json& jsonComponent) override;

	/// Used in the inspector to display different variables
	void DrawImGui(Entity entity) override;

protected:
	Entity selectedEntity_ = INVALID_ENTITY;
	Transform3dManager& transform3dManager_;
	RigidStaticData rigidStaticData_;
	PhysicsEngine& physicsEngine_;
	RigidStaticManager& rigidStaticManager_;
};

//-----------------------------------------------------------------------------
// Dynamic RigidBodies
//-----------------------------------------------------------------------------
/// Contains the velocity information
struct VelocityData
{
	Vec3f linearVelocity  = Vec3f::zero;
	Vec3f angularVelocity = Vec3f::zero;
};

/// Data used by Dynamic RigidBodies
struct RigidDynamicData : public RigidActorData
{
	RigidDynamicData()        = default;
	~RigidDynamicData()       = default;
	float linearDamping       = 0.0f;
	float angularDamping      = 0.0f;
	float mass                = 1.0f;
	bool useGravity           = true;
	bool isKinematic          = false;
	Vec3<bool> freezePosition = Vec3<bool>(false);
	Vec3<bool> freezeRotation = Vec3<bool>(false);
};

class RigidDynamic : public RigidActor
{
public:
	/// Init a RigidDynamic actor in physx scene
	/// \param physics the PxPhysics pointer of the physicsEngine
	/// \param rigidStatic the data used to generate the rigidStatic
	/// \param position initial position of the actor
	/// \param eulerAngle initial rotation of the actor
	void Init(const PhysicsEngine& physics,
		const RigidDynamicData& rigidDynamic,
		const Vec3f& position,
		const EulerAngles& eulerAngle);

	/// Apply a force to the RigidBody
	/// \param force Force to apply
	/// \param forceMode how force is apply
	void AddForce(
		const Vec3f& force, physx::PxForceMode::Enum forceMode = physx::PxForceMode::eFORCE) const;

	/// Add force at a relative position of the RigidBody
	/// \param force Force to apply
	/// \param position Relative position from the center
	void AddForceAtPosition(const Vec3f& force, const Vec3f& position) const;

	/// Apply a torque to the RigidBody
	/// \param torque Torque to apply
	/// \param forceMode How the force will be applied
	void AddRelativeTorque(
		const Vec3f& torque, physx::PxForceMode::Enum forceMode = physx::PxForceMode::eFORCE) const;

    /// Get the data of an actor <br>
    /// Must be called in FixedUpdate or when physics is not running
    [[nodiscard]] RigidDynamicData GetRigidDynamicData() const;
    [[nodiscard]] VelocityData GetVelocityData() const;

    [[nodiscard]] physx::PxRigidDynamic* GetPxRigidDynamic() const { return rigidActor_; }

    void SetPosition(const Vec3f& pos) const;
	void SetRotation(const Quaternion& rot) const;

	/// Set the data of an actor <br>
	/// Must be called in FixedUpdate or when physics is not running
	void SetRigidDynamicData(const RigidDynamicData& rigidDynamicData) const;

private:
	physx::PxRigidDynamic* rigidActor_ = nullptr;
};

/// Manager for Dynamic RigidBodies
class RigidDynamicManager final
   : public ComponentManager<RigidDynamic, EntityMask(ComponentType::RIGID_DYNAMIC)>,
	 public FixedUpdateInterface
{
public:
	RigidDynamicManager(EntityManager& entityManager,
		Transform3dManager& transform3dManager,
		PhysicsEngine& physicsEngine);

	void FixedUpdate(seconds dt) override;

    void DestroyComponent(Entity entity) override;

    /// Find the Entity from an actor
    Entity FindEntityFromActor(physx::PxActor* actor);

	/// Create a RigidDynamic Actor with the given RigidDynamicData
	void AddRigidDynamic(Entity entity, const RigidDynamicData& body);

	/// Apply a force to the RigidBody
	/// \param force Force to apply
	/// \param forceMode how force is apply
	void AddForce(Entity entity,
		const Vec3f& force,
		physx::PxForceMode::Enum forceMode = physx::PxForceMode::eFORCE) const;

	/// Add force at a relative position of the RigidBody
	/// \param force Force to apply
	/// \param position Relative position from the center
	void AddForceAtPosition(Entity entity, const Vec3f& force, const Vec3f& position) const;

	/// Apply a torque to the RigidBody
	/// \param torque Torque to apply
	/// \param forceMode How the force will be applied
	void AddRelativeTorque(Entity entity,
		const Vec3f& torque,
		physx::PxForceMode::Enum forceMode = physx::PxForceMode::eFORCE) const;

	/// Get the data of an actor <br>
	/// Must be called in FixedUpdate or when physics is not running
	[[nodiscard]] RigidDynamicData GetRigidDynamicData(Entity entity) const;
	[[nodiscard]] VelocityData GetVelocityData(Entity entity) const;

    void SetPosition(Entity entity, const Vec3f& pos) const;
    void SetRotation(Entity entity, const Quaternion& rot) const;

    void SetLinearVelocity(Entity entity, const Vec3f& linearVelocity) const;
    void SetAngularVelocity(Entity entity, const Vec3f& angularVelocity) const;

	/// Set the data of an actor <br>
	/// Must be called in FixedUpdate or when physics is not running
	void SetRigidDynamicData(Entity entity, const RigidDynamicData& rigidDynamicData) const;

protected:
	Transform3dManager& transform3dManager_;
	PhysicsEngine& physicsEngine_;
};

/// Used to serialize colliders and materials to / from json and imgui
class RigidDynamicSerializer final : public ComponentSerializer,
									 public FixedUpdateInterface,
									 public RigidActorSerializer
{
public:
	RigidDynamicSerializer(Transform3dManager& transform3dManager,
		EntityManager& entityManager,
		PhysicsEngine& physicsEngine,
		RigidDynamicManager& rigidDynamicManager);

	void SetSelectedEntity(Entity selectedEntity);

	void FixedUpdate(seconds dt) override;

    /// Get the json object from a RigidDynamic component
	[[nodiscard]] json GetJsonFromComponent(Entity entity) const override;

    /// Set a RigidDynamic component from a json object
	void SetComponentFromJson(Entity entity, const json& jsonComponent) override;

    /// Used in the inspector to display different variables
	void DrawImGui(Entity entity) override;

protected:
	Entity selectedEntity_     = INVALID_ENTITY;
	Entity lastSelectedEntity_ = INVALID_ENTITY;
	Transform3dManager& transform3dManager_;
	PhysicsEngine& physicsEngine_;
	VelocityData dynamicData_;
	RigidDynamicData rigidDynamicData_;
	RigidDynamicManager& rigidDynamicManager_;
};
}    // namespace neko::physics
