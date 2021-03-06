#include <imgui.h>

#include "engine/resource_locations.h"

#include "px/physics_engine.h"
#include "px/physx_utility.h"

#include "aer/tag.h"

namespace neko::physics
{
//-----------------------------------------------------------------------------
// RigidActor
//-----------------------------------------------------------------------------
ColliderType RigidActor::GetColliderType() const
{
	if (shape_ == nullptr) return ColliderType::INVALID;

	switch (shape_->getGeometryType())
	{
		case physx::PxGeometryType::eBOX: return ColliderType::BOX;
		case physx::PxGeometryType::eSPHERE: return ColliderType::SPHERE;
		case physx::PxGeometryType::eCAPSULE: return ColliderType::CAPSULE;
		case physx::PxGeometryType::eTRIANGLEMESH: return ColliderType::MESH;

		case physx::PxGeometryType::ePLANE:
		case physx::PxGeometryType::eCONVEXMESH:
		case physx::PxGeometryType::eHEIGHTFIELD:
		case physx::PxGeometryType::eGEOMETRY_COUNT:
		case physx::PxGeometryType::eINVALID:
		default: return ColliderType::INVALID;
	}
}

PhysicsMaterial RigidActor::GetPhysicsMaterial() const
{
	PhysicsMaterial physicsMaterial;
	if (!shape_)
	{
		LogDebug("No material found for this actor");
		return physicsMaterial;
	}

	physicsMaterial.bounciness      = material_->getRestitution();
	physicsMaterial.staticFriction  = material_->getStaticFriction();
	physicsMaterial.dynamicFriction = material_->getDynamicFriction();
	return physicsMaterial;
}

BoxColliderData RigidActor::GetBoxColliderData() const
{
	BoxColliderData boxColliderData;
	if (!shape_)
	{
		LogDebug("No box collider found for this actor");
		return boxColliderData;
	}

	boxColliderData.isTrigger =
		(shape_->getFlags() & physx::PxShapeFlags(physx::PxShapeFlag::eTRIGGER_SHAPE)) ==
		physx::PxShapeFlags(physx::PxShapeFlag::eTRIGGER_SHAPE);
	boxColliderData.offset = ConvertFromPxVec(shape_->getLocalPose().p);
	boxColliderData.size   = ConvertFromPxVec(shape_->getGeometry().box().halfExtents * 2.0f);
	return boxColliderData;
}

SphereColliderData RigidActor::GetSphereColliderData() const
{
	SphereColliderData sphereColliderData;
	if (!shape_)
	{
		LogDebug("No sphere collider found for this actor");
		return sphereColliderData;
	}

	sphereColliderData.isTrigger =
		(shape_->getFlags() & physx::PxShapeFlags(physx::PxShapeFlag::eTRIGGER_SHAPE)) ==
		physx::PxShapeFlags(physx::PxShapeFlag::eTRIGGER_SHAPE);
	sphereColliderData.offset = ConvertFromPxVec(shape_->getLocalPose().p);
	sphereColliderData.radius = shape_->getGeometry().sphere().radius;
	return sphereColliderData;
}

CapsuleColliderData RigidActor::GetCapsuleColliderData() const
{
	CapsuleColliderData capsuleColliderData;
	if (!shape_)
	{
		LogDebug("No capsule collider found for this actor");
		return capsuleColliderData;
	}

	capsuleColliderData.isTrigger =
		(shape_->getFlags() & physx::PxShapeFlags(physx::PxShapeFlag::eTRIGGER_SHAPE)) ==
		physx::PxShapeFlags(physx::PxShapeFlag::eTRIGGER_SHAPE);
	capsuleColliderData.offset = ConvertFromPxVec(shape_->getLocalPose().p);
	capsuleColliderData.height = shape_->getGeometry().capsule().halfHeight * 2.0f;
	capsuleColliderData.radius = shape_->getGeometry().capsule().radius;
	return capsuleColliderData;
}

void RigidActor::SetMaterial(const PhysicsMaterial& physicsMaterial) const
{
    material_->setRestitution(physicsMaterial.bounciness);
    material_->setStaticFriction(physicsMaterial.staticFriction);
    material_->setDynamicFriction(physicsMaterial.dynamicFriction);
    material_->setRestitutionCombineMode(physx::PxCombineMode::eMIN);
    material_->setFrictionCombineMode(physx::PxCombineMode::eMIN);
}

void RigidActor::SetBoxColliderData(const BoxColliderData& boxColliderData) const
{
    if (!shape_)
    {
        LogDebug("No box collider found for this actor");
        return;
    }

    physx::PxTransform transform = shape_->getLocalPose();
    transform.p                  = ConvertToPxVec(boxColliderData.offset);
    shape_->setLocalPose(transform);

    physx::PxBoxGeometry geometry = shape_->getGeometry().box();
    geometry.halfExtents          = ConvertToPxVec(boxColliderData.size / 2.0f);
    shape_->setGeometry(geometry);

    if (boxColliderData.isTrigger)
    {
        shape_->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
        shape_->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, boxColliderData.isTrigger);
    }
    else
    {
        shape_->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, boxColliderData.isTrigger);
        shape_->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
    }
}

void RigidActor::SetSphereColliderData(const SphereColliderData& sphereColliderData) const
{
	if (!shape_)
	{
        LogDebug("No sphere collider found for this actor");
		return;
	}

	physx::PxTransform transform = shape_->getLocalPose();
	transform.p                  = ConvertToPxVec(sphereColliderData.offset);
	shape_->setLocalPose(transform);

	physx::PxSphereGeometry geometry = shape_->getGeometry().sphere();
	geometry.radius                  = sphereColliderData.radius;
	shape_->setGeometry(geometry);
	shape_->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, sphereColliderData.isTrigger);
}

void RigidActor::SetCapsuleColliderData(const CapsuleColliderData& capsuleColliderData) const
{
	if (!shape_)
	{
        LogDebug("No capsule collider found for this actor");
		return;
	}

	physx::PxTransform transform = shape_->getLocalPose();
	transform.p                  = ConvertToPxVec(capsuleColliderData.offset);
	shape_->setLocalPose(transform);

	physx::PxCapsuleGeometry geometry = shape_->getGeometry().capsule();
	geometry.halfHeight               = capsuleColliderData.height / 2.0f;
	geometry.radius                   = capsuleColliderData.radius;
	shape_->setGeometry(geometry);

	if (capsuleColliderData.isTrigger)
	{
		shape_->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
		shape_->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, capsuleColliderData.isTrigger);
	}
	else
	{
		shape_->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, capsuleColliderData.isTrigger);
		shape_->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
	}
}

void RigidActor::SetMeshColliderData(const MeshColliderData& meshColliderData) const
{
	if (!shape_)
	{
        LogDebug("No mesh collider found for this actor");
		return;
	}

	physx::PxTransform transform = shape_->getLocalPose();
	transform.p                  = ConvertToPxVec(meshColliderData.offset);
	shape_->setLocalPose(transform);

	if (meshColliderData.isTrigger)
	{
		shape_->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
		shape_->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, meshColliderData.isTrigger);
	}
	else
	{
		shape_->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, meshColliderData.isTrigger);
		shape_->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
	}
}

physx::PxMaterial* RigidActor::InitMaterial(
	physx::PxPhysics* physics, const PhysicsMaterial& material)
{
	return physics->createMaterial(
		material.staticFriction, material.dynamicFriction, material.bounciness);
}

physx::PxShape* RigidActor::InitBoxShape(
	physx::PxPhysics* physics, physx::PxMaterial* material, const BoxColliderData& boxCollider)
{
	return physics->createShape(
		physx::PxBoxGeometry(boxCollider.size.x, boxCollider.size.y, boxCollider.size.z),
		*material);
}

physx::PxShape* RigidActor::InitSphereShape(physx::PxPhysics* physics,
	physx::PxMaterial* material,
	const SphereColliderData& sphereCollider)
{
	return physics->createShape(physx::PxSphereGeometry(sphereCollider.radius), *material);
}

physx::PxShape* RigidActor::InitCapsuleShape(physx::PxPhysics* physics,
	physx::PxMaterial* material,
	const CapsuleColliderData& capsuleCollider)
{
	return physics->createShape(
		physx::PxCapsuleGeometry(capsuleCollider.radius, capsuleCollider.height / 2.0f), *material);
}

physx::PxShape* RigidActor::InitMeshCollider(const PhysicsEngine& physics,
	physx::PxMaterial* material,
#ifdef NEKO_OPENGL
	const gl::Mesh& mesh,
#else
	const vk::Mesh& mesh,
#endif
	const physx::PxMeshScale& scale)
{
#ifdef NEKO_OPENGL
	std::vector<gl::Vertex> vertices   = mesh.GetVertices();
	std::vector<std::uint32_t> indices = mesh.GetIndices();
	std::vector<physx::PxVec3> pxVertices;
	pxVertices.resize(vertices.size());
	std::transform(vertices.begin(),
		vertices.end(),
		pxVertices.begin(),
		[](gl::Vertex vert) -> physx::PxVec3 { return ConvertToPxVec(vert.position); });
#elif NEKO_VULKAN
	std::vector<vk::Vertex> vertices = mesh.GetVertices(0);
	std::vector<unsigned> indices    = mesh.GetIndices(0);
	std::vector<physx::PxVec3> pxVertices;
	pxVertices.resize(vertices.size());
	std::transform(vertices.begin(),
		vertices.end(),
		pxVertices.begin(),
		[](vk::Vertex vert) -> physx::PxVec3 { return ConvertToPxVec(vert.position); });
#endif
	physx::PxTriangleMeshDesc meshDesc;
	meshDesc.points.count  = pxVertices.size();
	meshDesc.points.stride = sizeof(physx::PxVec3);
	meshDesc.points.data   = pxVertices.data();

	meshDesc.triangles.count  = indices.size() / 3;
	meshDesc.triangles.stride = 3 * sizeof(physx::PxU32);
	meshDesc.triangles.data   = indices.data();

	physx::PxDefaultMemoryOutputStream writeBuffer;
	physx::PxTriangleMeshCookingResult::Enum result;
	bool status = physics.GetCooking()->cookTriangleMesh(meshDesc, writeBuffer, &result);
	if (!status)
	{
		switch (result)
		{
			case physx::PxTriangleMeshCookingResult::eSUCCESS: break;
			case physx::PxTriangleMeshCookingResult::eLARGE_TRIANGLE:
				LogDebug("eLARGE_TRIANGLE");
				break;
			case physx::PxTriangleMeshCookingResult::eFAILURE: LogDebug("eFAILURE"); break;
			default:;
		}
	}

	physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
	physx::PxTriangleMesh* triangleMesh = physics.GetPhysx()->createTriangleMesh(readBuffer);
	return physics.GetPhysx()->createShape(
		physx::PxTriangleMeshGeometry(triangleMesh, scale), *material);
}

void RigidActor::SetFiltering(physx::PxShape* shape, physx::PxU32 filterGroup)
{
	physx::PxFilterData filterData;
	filterData.word0 = filterGroup;    // word0 = own ID
	filterData.word1 = FilterGroup::EVERYTHING;

	// contact callback;
	shape->setSimulationFilterData(filterData);
	shape->setQueryFilterData(filterData);
}

//-----------------------------------------------------------------------------
// RigidActorSerializer
//-----------------------------------------------------------------------------
json RigidActorSerializer::GetJsonFromMaterial(const RigidActorData& rigidActorData)
{
    json materialJson               = json::object();
    materialJson["exist"]           = rigidActorData.colliderType != ColliderType::INVALID;
    materialJson["bounciness"]      = rigidActorData.material.bounciness;
    materialJson["staticFriction"]  = rigidActorData.material.staticFriction;
    materialJson["dynamicFriction"] = rigidActorData.material.dynamicFriction;
    return materialJson;
}

json RigidActorSerializer::GetJsonFromBoxCollider(const RigidActorData& rigidActorData)
{
	json colliderJson         = json::object();
	colliderJson["exist"]     = rigidActorData.colliderType == ColliderType::BOX;
	colliderJson["isTrigger"] = rigidActorData.boxColliderData.isTrigger;
	colliderJson["offset"]    = GetJsonFromVector3(rigidActorData.boxColliderData.offset);
	colliderJson["size"]      = GetJsonFromVector3(rigidActorData.boxColliderData.size);
	return colliderJson;
}

json RigidActorSerializer::GetJsonFromCapsuleCollider(const RigidActorData& rigidActorData)
{
	json colliderJson         = json::object();
	colliderJson["exist"]     = rigidActorData.colliderType == ColliderType::CAPSULE;
	colliderJson["isTrigger"] = rigidActorData.capsuleColliderData.isTrigger;
	colliderJson["offset"]    = GetJsonFromVector3(rigidActorData.capsuleColliderData.offset);
	colliderJson["height"]    = rigidActorData.capsuleColliderData.height;
	colliderJson["radius"]    = rigidActorData.capsuleColliderData.radius;
	return colliderJson;
}

json RigidActorSerializer::GetJsonFromSphereCollider(const RigidActorData& rigidActorData)
{
	json colliderJson         = json::object();
	colliderJson["exist"]     = rigidActorData.colliderType == ColliderType::SPHERE;
	colliderJson["isTrigger"] = rigidActorData.sphereColliderData.isTrigger;
	colliderJson["offset"]    = GetJsonFromVector3(rigidActorData.sphereColliderData.offset);
	colliderJson["radius"]    = rigidActorData.sphereColliderData.radius;
	return colliderJson;
}

RigidActorData RigidActorSerializer::SetFromJson(const json& rigidActorJson)
{
	RigidActorData rigidActorData;
	if (CheckJsonParameter(rigidActorJson, "sphereCollider", json::value_t::object))
	{
		if (CheckJsonParameter(rigidActorJson["sphereCollider"], "exist", json::value_t::boolean))
		{
			const json& sphereJson = rigidActorJson["sphereCollider"];
			if (sphereJson["exist"])
			{
				rigidActorData.colliderType                 = ColliderType::SPHERE;
				rigidActorData.sphereColliderData.isTrigger = sphereJson["isTrigger"].get<bool>();
				rigidActorData.sphereColliderData.offset = GetVector3FromJson(sphereJson, "offset");
				rigidActorData.sphereColliderData.radius = sphereJson["radius"].get<float>();
			}
		}
	}

	if (CheckJsonParameter(rigidActorJson, "boxCollider", json::value_t::object))
	{
		if (CheckJsonParameter(rigidActorJson["boxCollider"], "exist", json::value_t::boolean))
		{
			const json& boxJson = rigidActorJson["boxCollider"];
			if (boxJson["exist"])
			{
				rigidActorData.colliderType              = ColliderType::BOX;
				rigidActorData.boxColliderData.isTrigger = boxJson["isTrigger"].get<bool>();
				rigidActorData.boxColliderData.offset    = GetVector3FromJson(boxJson, "offset");
				rigidActorData.boxColliderData.size      = GetVector3FromJson(boxJson, "size");
			}
		}
	}

	if (CheckJsonParameter(rigidActorJson, "capsuleCollider", json::value_t::object))
	{
		if (CheckJsonParameter(rigidActorJson["capsuleCollider"], "exist", json::value_t::boolean))
		{
			const json& capsuleJson = rigidActorJson["capsuleCollider"];
			if (capsuleJson["exist"])
			{
				rigidActorData.colliderType                  = ColliderType::CAPSULE;
				rigidActorData.capsuleColliderData.isTrigger = capsuleJson["isTrigger"].get<bool>();
				rigidActorData.capsuleColliderData.offset =
					GetVector3FromJson(capsuleJson, "offset");
				rigidActorData.capsuleColliderData.height = capsuleJson["height"].get<float>();
				rigidActorData.capsuleColliderData.radius = capsuleJson["radius"].get<float>();
			}
		}
	}

	if (CheckJsonParameter(rigidActorJson, "physicsMaterial", json::value_t::object))
	{
		const json& materialJson                = rigidActorJson["physicsMaterial"];
		rigidActorData.material.bounciness      = materialJson["bounciness"];
		rigidActorData.material.staticFriction  = materialJson["staticFriction"];
		rigidActorData.material.dynamicFriction = materialJson["dynamicFriction"];
	}

	return rigidActorData;
}

RigidActorData RigidActorSerializer::DrawActorImGui(const RigidActorData& rigidActorData)
{
	RigidActorData newData = rigidActorData;
	switch (newData.colliderType)
	{
		case ColliderType::INVALID: break;
		case ColliderType::BOX:
		{
			BoxColliderData boxColliderData = newData.boxColliderData;
			if (ImGui::CollapsingHeader("Box Collider", true))
			{
				ImGui::DragFloat3("Offset", boxColliderData.offset.coord);
				ImGui::DragFloat3("Size", boxColliderData.size.coord, 0.1f, 0.0f);
				ImGui::Checkbox("Is Trigger", &boxColliderData.isTrigger);
				newData.boxColliderData = boxColliderData;
			}
			break;
		}
		case ColliderType::SPHERE:
		{
			SphereColliderData sphereColliderData = newData.sphereColliderData;
			if (ImGui::CollapsingHeader("Sphere Collider", true))
			{
				ImGui::DragFloat3("Offset", sphereColliderData.offset.coord, 0);
				ImGui::DragFloat("Radius", &sphereColliderData.radius);
				ImGui::Checkbox("Is Trigger", &sphereColliderData.isTrigger);
				newData.sphereColliderData = sphereColliderData;
			}
			break;
		}
		case ColliderType::CAPSULE:
		{
			CapsuleColliderData capsuleColliderData = newData.capsuleColliderData;
			if (ImGui::CollapsingHeader("Sphere Collider", true))
			{
				ImGui::DragFloat3("Offset", capsuleColliderData.offset.coord, 0);
				ImGui::DragFloat("Radius", &capsuleColliderData.radius);
				ImGui::DragFloat("Height", &capsuleColliderData.height);
				ImGui::Checkbox("Is Trigger", &capsuleColliderData.isTrigger);
				newData.capsuleColliderData = capsuleColliderData;
			}
			break;
		}
		case ColliderType::MESH:
		{
			MeshColliderData meshColliderData = newData.meshColliderData;
			if (ImGui::CollapsingHeader("Mesh Collider", true))
			{
				ImGui::DragFloat3("Offset", meshColliderData.offset.coord, 0);
				ImGui::Checkbox("Is Trigger", &meshColliderData.isTrigger);
				newData.meshColliderData = meshColliderData;
			}
			break;
		}
		default:; break;
	}

	if (ImGui::CollapsingHeader("Material", true))
	{
		ImGui::DragFloat("bounciness", &newData.material.bounciness, 0.1f, 0.0f, 1.0f);
		ImGui::DragFloat("staticFriction", &newData.material.staticFriction, 0.1f, 0.0f, 1.0f);
		ImGui::DragFloat("dynamicFriction", &newData.material.dynamicFriction, 0.1f, 0.0f, 1.0f);
	}

	return newData;
}

//-----------------------------------------------------------------------------
// RigidStatic
//-----------------------------------------------------------------------------
void RigidStatic::Init(const PhysicsEngine& physics,
	const RigidStaticData& rigidStatic,
	const Vec3f& position,
	const EulerAngles& eulerAngle)
{
	physx::PxTransform transform = physx::PxTransform(
		ConvertToPxVec(position), ConvertToPxQuat(Quaternion::FromEuler(eulerAngle)));
	rigidActor_ = physics.GetPhysx()->createRigidStatic(transform);
	if (!rigidActor_) LogError("Couldn't create actor!");

	material_ = InitMaterial(physics.GetPhysx(), rigidStatic.material);
	if (!material_) LogError("Couldn't create material!");

	if (rigidStatic.colliderType == ColliderType::MESH)
	{
#ifdef NEKO_OPENGL
		const auto& modelManager = gl::ModelManagerLocator::get();
		if (modelManager.IsLoaded(rigidStatic.meshColliderData.modelId))
		{
			const auto& model = modelManager.GetModel(rigidStatic.meshColliderData.modelId);
			for (size_t meshIndex = 0; meshIndex < model->GetMeshCount(); ++meshIndex)
			{
				shape_ = InitMeshCollider(physics,
					material_,
					model->GetMesh(meshIndex),
					physx::PxMeshScale(rigidStatic.meshColliderData.size));
				if (!shape_)
				{
                    LogError("Couldn't create actor shape!");
					return;
				}

				SetFiltering(shape_, rigidStatic.filterGroup);
				rigidActor_->attachShape(*shape_);
				SetRigidStaticData(rigidStatic);
			}
		}
#elif NEKO_VULKAN
		const auto& modelManager = vk::ModelManagerLocator::get();
		if (modelManager.IsLoaded(rigidStatic.meshColliderData.modelId))
		{
			const auto& model = modelManager.GetModel(rigidStatic.meshColliderData.modelId);
			for (size_t meshIndex = 0; meshIndex < model->GetMeshCount(); ++meshIndex)
			{
				shape_ = InitMeshCollider(physics,
					material_,
					model->GetMesh(meshIndex),
					physx::PxMeshScale(rigidStatic.meshColliderData.size));
				if (!shape_)
				{
					std::cerr << "createShape failed!";
					return;
				}

				SetFiltering(shape_, rigidStatic.filterGroup);
				rigidActor_->attachShape(*shape_);
				SetRigidStaticData(rigidStatic);
			}
		}
#endif
	}
	else
	{
		switch (rigidStatic.colliderType)
		{
			case ColliderType::INVALID:
			case ColliderType::BOX:
				shape_ = InitBoxShape(physics.GetPhysx(), material_, rigidStatic.boxColliderData);
				break;
			case ColliderType::SPHERE:
				shape_ =
					InitSphereShape(physics.GetPhysx(), material_, rigidStatic.sphereColliderData);
				break;
			case ColliderType::CAPSULE:
				shape_ = InitCapsuleShape(
					physics.GetPhysx(), material_, rigidStatic.capsuleColliderData);
				break;
			default: break;
		}

		if (!shape_)
		{
            LogError("Couldn't create actor shape!");
			return;
		}

		SetFiltering(shape_, rigidStatic.filterGroup);
		rigidActor_->attachShape(*shape_);
		SetRigidStaticData(rigidStatic);
	}
}

RigidStaticData RigidStatic::GetRigidStaticData() const
{
    RigidStaticData rigidStaticData;
    if (!rigidActor_)
    {
        LogDebug("No rigidbody found for this actor");
        return rigidStaticData;
    }

    rigidStaticData.material     = GetPhysicsMaterial();
    rigidStaticData.colliderType = GetColliderType();
    switch (rigidStaticData.colliderType)
    {
        case ColliderType::INVALID: break;
        case ColliderType::BOX:
            rigidActor_->detachShape(*shape_);
            rigidStaticData.boxColliderData = GetBoxColliderData();
            rigidActor_->attachShape(*shape_);
            break;
        case ColliderType::SPHERE:
            rigidActor_->detachShape(*shape_);
            rigidStaticData.sphereColliderData = GetSphereColliderData();
            rigidActor_->attachShape(*shape_);
            break;
        case ColliderType::CAPSULE:
            rigidActor_->detachShape(*shape_);
            rigidStaticData.capsuleColliderData = GetCapsuleColliderData();
            rigidActor_->attachShape(*shape_);
            break;
        default: break;
    }

    return rigidStaticData;
}

void RigidStatic::SetRigidStaticData(const RigidStaticData& rigidStaticData) const
{
	if (!rigidActor_)
	{
        LogError("No rigidbody found for this actor");
		return;
	}

	SetMaterial(rigidStaticData.material);
	switch (rigidStaticData.colliderType)
	{
		case ColliderType::INVALID: break;
		case ColliderType::BOX:
			rigidActor_->detachShape(*shape_);
			SetBoxColliderData(rigidStaticData.boxColliderData);
			rigidActor_->attachShape(*shape_);
			break;
		case ColliderType::SPHERE:
			rigidActor_->detachShape(*shape_);
			SetSphereColliderData(rigidStaticData.sphereColliderData);
			rigidActor_->attachShape(*shape_);
			break;
		case ColliderType::CAPSULE:
			rigidActor_->detachShape(*shape_);
			SetCapsuleColliderData(rigidStaticData.capsuleColliderData);
			rigidActor_->attachShape(*shape_);
			break;
		case ColliderType::MESH:
			rigidActor_->detachShape(*shape_);
			SetMeshColliderData(rigidStaticData.meshColliderData);
			rigidActor_->attachShape(*shape_);
			break;
		default:;
	}
}

//-----------------------------------------------------------------------------
// RigidStaticManager
//-----------------------------------------------------------------------------
RigidStaticManager::RigidStaticManager(EntityManager& entityManager,
	Transform3dManager& transform3dManager,
	aer::RenderManager& renderManager,
	PhysicsEngine& physicsEngine)
   : ComponentManager<RigidStatic, EntityMask(ComponentType::RIGID_STATIC)>(entityManager),
	 transform3dManager_(transform3dManager),
	 physicsEngine_(physicsEngine),
	 renderManager_(renderManager)
{}

void RigidStaticManager::FixedUpdate(seconds dt)
{
	if (meshColliderToCreate_.empty()) return;

	for (auto& toCreate : meshColliderToCreate_)
	{
#ifdef NEKO_OPENGL
		if (gl::ModelManagerLocator::get().IsLoaded(toCreate.second))
#else
		if (vk::ModelManagerLocator::get().IsLoaded(toCreate.second))
#endif
		{
			RigidStaticData rigidStatic;
			rigidStatic.colliderType             = ColliderType::MESH;
			rigidStatic.meshColliderData.modelId = toCreate.second;
			rigidStatic.meshColliderData.size    = 100.0f;

			std::string layer = aer::TagLocator::get().GetEntityLayer(toCreate.first);
			if (layer == "Ground")
				rigidStatic.filterGroup = FilterGroup::GROUND;
			else if (layer == "Ship")
				rigidStatic.filterGroup = FilterGroup::SHIP;
			else if (layer == "Wall")
				rigidStatic.filterGroup = FilterGroup::WALL;
			else
				rigidStatic.filterGroup = FilterGroup::DEFAULT;
			AddRigidStatic(toCreate.first, rigidStatic);
			meshColliderToCreate_.erase(toCreate.first);

			if (meshColliderToCreate_.empty()) return;
		}
	}
}

void RigidStaticManager::DestroyComponent(Entity entity)
{
	if (entity < components_.size())
	{
		if (GetComponent(entity).GetPxRigidStatic())
			physicsEngine_.GetScene()->removeActor(*GetComponent(entity).GetPxRigidStatic());
		SetComponent(entity, RigidStatic());
	}

	ComponentManager::DestroyComponent(entity);
}

Entity RigidStaticManager::FindEntityFromActor(physx::PxActor* actor)
{
	auto entityIt = std::find_if(components_.begin(),
		components_.end(),
		[actor](RigidStatic rigidStatic) { return actor == rigidStatic.GetPxRigidStatic(); });
	if (entityIt == components_.end()) return INVALID_ENTITY;

	return std::distance(components_.begin(), entityIt);
}

void RigidStaticManager::AddRigidStatic(Entity entity, const RigidStaticData& rigidStaticData)
{
	AddComponent(entity);
	const Vec3f position    = transform3dManager_.GetGlobalPosition(entity);
	const EulerAngles euler = transform3dManager_.GetGlobalRotation(entity);
	const Vec3f scale       = transform3dManager_.GetGlobalScale(entity);

	RigidStaticData newRigidStaticData = rigidStaticData;
	newRigidStaticData.boxColliderData.size =
		Vec3f(newRigidStaticData.boxColliderData.size.x * scale.x,
			newRigidStaticData.boxColliderData.size.y * scale.y,
			newRigidStaticData.boxColliderData.size.z * scale.z);
	newRigidStaticData.sphereColliderData.radius =
		newRigidStaticData.sphereColliderData.radius * scale.x;
	newRigidStaticData.capsuleColliderData.height =
		newRigidStaticData.capsuleColliderData.height * scale.x;
	newRigidStaticData.capsuleColliderData.radius =
		newRigidStaticData.capsuleColliderData.radius * scale.z;
	newRigidStaticData.capsuleColliderData.offset =
		newRigidStaticData.capsuleColliderData.offset * scale;
	newRigidStaticData.sphereColliderData.offset =
		newRigidStaticData.sphereColliderData.offset * scale;
	newRigidStaticData.boxColliderData.offset = newRigidStaticData.boxColliderData.offset * scale;
	newRigidStaticData.meshColliderData.size  = newRigidStaticData.meshColliderData.size * scale.x;

	RigidStatic rigidStatic = GetComponent(entity);
	rigidStatic.Init(physicsEngine_, newRigidStaticData, position, euler);
	physicsEngine_.GetScene()->addActor(*rigidStatic.GetPxRigidStatic());
	SetComponent(entity, rigidStatic);
}

void RigidStaticManager::AddMeshColliderStatic(Entity entity, std::string_view modelName)
{
	const std::string modelPath =
		GetModelsFolderPath() + modelName.data() + "/" + modelName.data() + ".obj";

#ifdef NEKO_OPENGL
	gl::ModelId modelId = gl::ModelManagerLocator::get().LoadModel(modelPath);
	if (gl::ModelManagerLocator::get().IsLoaded(modelId))
#else
	vk::ModelId modelId = vk::ModelManagerLocator::get().LoadModel(modelPath);
	if (vk::ModelManagerLocator::get().IsLoaded(modelId))
#endif
	{
		RigidStaticData rigidStatic;
		rigidStatic.colliderType             = ColliderType::MESH;
		rigidStatic.meshColliderData.modelId = modelId;
		rigidStatic.meshColliderData.size    = 100.0f;

		std::string layer = aer::TagLocator::get().GetEntityLayer(entity);
		if (layer == "Ground")
			rigidStatic.filterGroup = FilterGroup::GROUND;
		else if (layer == "Ship")
			rigidStatic.filterGroup = FilterGroup::SHIP;
		else if (layer == "Wall")
			rigidStatic.filterGroup = FilterGroup::WALL;
		else
			rigidStatic.filterGroup = FilterGroup::DEFAULT;

		AddRigidStatic(entity, rigidStatic);
	}
	else
	{
		meshColliderToCreate_.emplace(entity, modelId);
	}
}

const RigidStaticData& RigidStaticManager::GetRigidStaticData(Entity entity) const
{
	return GetComponent(entity).GetRigidStaticData();
}

void RigidStaticManager::SetRigidStaticData(
	Entity entity, const RigidStaticData& rigidStaticData) const
{
	if (!physicsEngine_.IsPhysicRunning())
	{
		GetComponent(entity).SetRigidStaticData(rigidStaticData);
	}
}

//-----------------------------------------------------------------------------
// RigidStaticSerializer
//-----------------------------------------------------------------------------
RigidStaticSerializer::RigidStaticSerializer(Transform3dManager& transform3dManager,
	EntityManager& entityManager,
	PhysicsEngine& physicsEngine,
	RigidStaticManager& rigidStaticManager)
   : ComponentSerializer(entityManager),
	 physicsEngine_(physicsEngine),
	 rigidStaticManager_(rigidStaticManager),
	 transform3dManager_(transform3dManager)
{}

void RigidStaticSerializer::SetSelectedEntity(Entity selectedEntity)
{
    selectedEntity_ = selectedEntity;
    if (selectedEntity_ == INVALID_ENTITY) return;
    rigidStaticData_ = rigidStaticManager_.GetRigidStaticData(selectedEntity_);
}

json RigidStaticSerializer::GetJsonFromComponent(Entity entity) const
{
	json rigidDynamicViewer = json::object();
	if (entityManager_.HasComponent(entity, EntityMask(ComponentType::RIGID_STATIC)))
	{
		if (entity != INVALID_ENTITY && entityManager_.GetEntitiesSize() > entity)
		{
			RigidStaticData rigidStaticData = rigidStaticManager_.GetRigidStaticData(entity);
			Vec3f scale                     = transform3dManager_.GetGlobalScale(entity);
			rigidStaticData.boxColliderData.size =
				Vec3f(rigidStaticData.boxColliderData.size.x / scale.x,
					rigidStaticData.boxColliderData.size.y / scale.y,
					rigidStaticData.boxColliderData.size.z / scale.z);
			rigidStaticData.sphereColliderData.radius =
				rigidStaticData.sphereColliderData.radius / scale.x;
			rigidStaticData.capsuleColliderData.height =
				rigidStaticData.capsuleColliderData.height / scale.x;
			rigidStaticData.capsuleColliderData.radius =
				rigidStaticData.capsuleColliderData.radius / scale.z;

			RigidDynamicData rigidDynamicData;
			rigidDynamicViewer["useGravity"]        = rigidDynamicData.useGravity;
			rigidDynamicViewer["isKinematic"]       = rigidDynamicData.isKinematic;
			rigidDynamicViewer["isStatic"]          = true;
			rigidDynamicViewer["mass"]              = rigidDynamicData.mass;
			rigidDynamicViewer["linearDamping"]     = rigidDynamicData.linearDamping;
			rigidDynamicViewer["angularDamping"]    = rigidDynamicData.angularDamping;
			rigidDynamicViewer["rotationLock"]      = json::object();
			rigidDynamicViewer["rotationLock"]["x"] = rigidDynamicData.freezeRotation.x;
			rigidDynamicViewer["rotationLock"]["y"] = rigidDynamicData.freezeRotation.y;
			rigidDynamicViewer["rotationLock"]["z"] = rigidDynamicData.freezeRotation.z;
			rigidDynamicViewer["positionLock"]      = json::object();
			rigidDynamicViewer["positionLock"]["x"] = rigidDynamicData.freezePosition.x;
			rigidDynamicViewer["positionLock"]["y"] = rigidDynamicData.freezePosition.y;
			rigidDynamicViewer["positionLock"]["z"] = rigidDynamicData.freezePosition.z;
			rigidDynamicViewer["boxCollider"]       = GetJsonFromBoxCollider(rigidStaticData);
			rigidDynamicViewer["sphereCollider"]    = GetJsonFromSphereCollider(rigidStaticData);
			rigidDynamicViewer["capsuleCollider"]   = GetJsonFromCapsuleCollider(rigidStaticData);
			rigidDynamicViewer["physicsMaterial"]   = GetJsonFromMaterial(rigidStaticData);
		}
	}

	return rigidDynamicViewer;
}

void RigidStaticSerializer::SetComponentFromJson(Entity entity, const json& componentJson)
{
	RigidStaticData rigidStaticData;
	RigidActorData rigidActorData       = SetFromJson(componentJson);
	rigidStaticData.colliderType        = rigidActorData.colliderType;
	rigidStaticData.boxColliderData     = rigidActorData.boxColliderData;
	rigidStaticData.sphereColliderData  = rigidActorData.sphereColliderData;
	rigidStaticData.capsuleColliderData = rigidActorData.capsuleColliderData;
	rigidStaticData.material            = rigidActorData.material;

	std::string layer = aer::TagLocator::get().GetEntityLayer(entity);
	if (layer == "Ground")
		rigidStaticData.filterGroup = FilterGroup::GROUND;
	else if (layer == "Ship")
		rigidStaticData.filterGroup = FilterGroup::SHIP;
	else if (layer == "Wall")
		rigidStaticData.filterGroup = FilterGroup::WALL;
	else
		rigidStaticData.filterGroup = FilterGroup::DEFAULT;

	rigidStaticManager_.AddRigidStatic(entity, rigidStaticData);
}

void RigidStaticSerializer::DrawImGui(Entity entity)
{
	if (entity == INVALID_ENTITY) return;
	if (entityManager_.HasComponent(entity, EntityMask(ComponentType::RIGID_STATIC)))
	{
		if (ImGui::TreeNode("Rigid Static"))
		{
			SetSelectedEntity(entity);
			RigidStaticData rigidStaticData     = rigidStaticData_;
			RigidActorData rigidActorData       = DrawActorImGui(rigidStaticData);
			rigidStaticData.material            = rigidActorData.material;
			rigidStaticData.boxColliderData     = rigidActorData.boxColliderData;
			rigidStaticData.sphereColliderData  = rigidActorData.sphereColliderData;
			rigidStaticData.capsuleColliderData = rigidActorData.capsuleColliderData;
			if (!physicsEngine_.IsPhysicRunning())
			{
				rigidStaticManager_.SetRigidStaticData(selectedEntity_, rigidStaticData);
				rigidStaticData_ = rigidStaticManager_.GetRigidStaticData(selectedEntity_);
			}
			ImGui::TreePop();
		}
	}
}

//-----------------------------------------------------------------------------
// RigidDynamic
//-----------------------------------------------------------------------------
void RigidDynamic::Init(const PhysicsEngine& physics,
	const RigidDynamicData& rigidDynamic,
	const Vec3f& position,
	const EulerAngles& eulerAngle)
{
	physx::PxTransform transform = physx::PxTransform(
		ConvertToPxVec(position), ConvertToPxQuat(Quaternion::FromEuler(eulerAngle)));
	rigidActor_ = physics.GetPhysx()->createRigidDynamic(transform);
	if (!rigidActor_)
	{
        LogError("Couldn't create actor!");
		return;
	}

	material_ = InitMaterial(physics.GetPhysx(), rigidDynamic.material);
	if (!material_)
	{
        LogError("Couldn't create material!");
		return;
	}

	switch (rigidDynamic.colliderType)
    {
		case ColliderType::INVALID: break;
		case ColliderType::BOX:
			shape_ = InitBoxShape(physics.GetPhysx(), material_, rigidDynamic.boxColliderData);
			break;
		case ColliderType::SPHERE:
			shape_ =
				InitSphereShape(physics.GetPhysx(), material_, rigidDynamic.sphereColliderData);
			break;
		case ColliderType::CAPSULE:
			shape_ =
				InitCapsuleShape(physics.GetPhysx(), material_, rigidDynamic.capsuleColliderData);
			break;
		case ColliderType::MESH: LogError("Mesh collider aren't supported yet!"); break;
		default:;
	}

	if (!shape_)
    {
        LogError("Couldn't create dynamic rigidbody!");
        return;
    }

    rigidActor_->attachShape(*shape_);

    const auto posIt = physx::PxU32(10);
    const auto velIt = physx::PxU32(1);
    rigidActor_->setSolverIterationCounts(posIt, velIt);
    SetRigidDynamicData(rigidDynamic);
}

void RigidDynamic::AddForce(const Vec3f& force, physx::PxForceMode::Enum forceMode) const
{
    if (!rigidActor_)
    {
        LogError("No rigidbody found for this actor!");
        return;
    }

    rigidActor_->addForce(ConvertToPxVec(force), forceMode);
}

void RigidDynamic::AddForceAtPosition(const Vec3f& force, const Vec3f& position) const
{
    if (!rigidActor_)
    {
        LogError("No rigidbody found for this actor!");
        return;
    }

    physx::PxRigidBodyExt::addForceAtLocalPos(
        *rigidActor_, ConvertToPxVec(force), ConvertToPxVec(position));
}

void RigidDynamic::AddRelativeTorque(const Vec3f& torque, physx::PxForceMode::Enum forceMode) const
{
    if (!rigidActor_)
    {
        LogError("No rigidbody found for this actor!");
        return;
    }

    rigidActor_->addTorque(ConvertToPxVec(torque), forceMode);
}

RigidDynamicData RigidDynamic::GetRigidDynamicData() const
{
	RigidDynamicData rigidDynamicData;
	if (!rigidActor_)
	{
		LogError("No rigidbody found for this actor!");
		return rigidDynamicData;
	}

	rigidDynamicData.linearDamping  = rigidActor_->getLinearDamping();
	rigidDynamicData.angularDamping = rigidActor_->getAngularDamping();
	rigidDynamicData.mass           = rigidActor_->getMass();
	rigidDynamicData.useGravity     = (rigidActor_->getActorFlags() &
                                      physx::PxActorFlags(physx::PxActorFlag::eDISABLE_GRAVITY)) !=
	                              physx::PxActorFlags(physx::PxActorFlag::eDISABLE_GRAVITY);
	rigidDynamicData.isKinematic =
		(rigidActor_->getRigidBodyFlags() &
			physx::PxRigidBodyFlags(physx::PxRigidBodyFlag::eKINEMATIC)) ==
		physx::PxRigidBodyFlags(physx::PxRigidBodyFlag::eKINEMATIC);
	rigidDynamicData.freezeRotation = {
		(rigidActor_->getRigidDynamicLockFlags() &
			physx::PxRigidDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X)) ==
			physx::PxRigidDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X),
		(rigidActor_->getRigidDynamicLockFlags() &
			physx::PxRigidDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y)) ==
			physx::PxRigidDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y),
		(rigidActor_->getRigidDynamicLockFlags() &
			physx::PxRigidDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z)) ==
			physx::PxRigidDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z)};
	rigidDynamicData.freezePosition = {
		(rigidActor_->getRigidDynamicLockFlags() &
			physx::PxRigidDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X)) ==
			physx::PxRigidDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X),
		(rigidActor_->getRigidDynamicLockFlags() &
			physx::PxRigidDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y)) ==
			physx::PxRigidDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y),
		(rigidActor_->getRigidDynamicLockFlags() &
			physx::PxRigidDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z)) ==
			physx::PxRigidDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z)};
	rigidDynamicData.material     = GetPhysicsMaterial();
	rigidDynamicData.colliderType = GetColliderType();
	switch (rigidDynamicData.colliderType)
	{
		case ColliderType::INVALID: break;
		case ColliderType::BOX:
			rigidActor_->detachShape(*shape_);
			rigidDynamicData.boxColliderData = GetBoxColliderData();
			rigidActor_->attachShape(*shape_);
			break;
		case ColliderType::SPHERE:
			rigidActor_->detachShape(*shape_);
			rigidDynamicData.sphereColliderData = GetSphereColliderData();
			rigidActor_->attachShape(*shape_);
			break;
		case ColliderType::CAPSULE:
			rigidActor_->detachShape(*shape_);
			rigidDynamicData.capsuleColliderData = GetCapsuleColliderData();
			rigidActor_->attachShape(*shape_);
			break;
		default:;
	}

	return rigidDynamicData;
}

VelocityData RigidDynamic::GetVelocityData() const
{
	VelocityData dynamicData;
	if (!rigidActor_)
	{
		LogError("No actor found!");
		return dynamicData;
	}

	dynamicData.linearVelocity  = ConvertFromPxVec(rigidActor_->getLinearVelocity());
	dynamicData.angularVelocity = ConvertFromPxVec(rigidActor_->getAngularVelocity());
	return dynamicData;
}

void RigidDynamic::SetPosition(const Vec3f& pos) const
{
    if (!rigidActor_)
    {
        LogError("No actor found!");
        return;
    }

    physx::PxTransform transform = rigidActor_->getGlobalPose();
    transform.p                  = ConvertToPxVec(pos);
    rigidActor_->setGlobalPose(transform);
}

void RigidDynamic::SetRotation(const Quaternion& rot) const
{
    if (!rigidActor_)
    {
        LogError("No actor found!");
		return;
	}

	physx::PxTransform transform = rigidActor_->getGlobalPose();
	transform.q                  = ConvertToPxQuat(rot);
	rigidActor_->setGlobalPose(transform);
}

void RigidDynamic::SetRigidDynamicData(const RigidDynamicData& rigidDynamicData) const
{
	if (!rigidActor_)
	{
		LogError("No rigidbody found for this actor!");
		return;
	}

	rigidActor_->setLinearDamping(rigidDynamicData.linearDamping);
	rigidActor_->setAngularDamping(rigidDynamicData.angularDamping);
	rigidActor_->setMass(rigidDynamicData.mass);
	rigidActor_->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, !rigidDynamicData.useGravity);
	rigidActor_->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, rigidDynamicData.isKinematic);
	rigidActor_->setRigidDynamicLockFlag(
		physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, rigidDynamicData.freezeRotation.x);
	rigidActor_->setRigidDynamicLockFlag(
		physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, rigidDynamicData.freezeRotation.y);
	rigidActor_->setRigidDynamicLockFlag(
		physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, rigidDynamicData.freezeRotation.z);
	rigidActor_->setRigidDynamicLockFlag(
		physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X, rigidDynamicData.freezePosition.x);
	rigidActor_->setRigidDynamicLockFlag(
		physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y, rigidDynamicData.freezePosition.y);
	rigidActor_->setRigidDynamicLockFlag(
		physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, rigidDynamicData.freezePosition.z);
	SetMaterial(rigidDynamicData.material);
	switch (rigidDynamicData.colliderType)
	{
		case ColliderType::INVALID: break;
		case ColliderType::BOX:
			rigidActor_->detachShape(*shape_);
			SetBoxColliderData(rigidDynamicData.boxColliderData);
			rigidActor_->attachShape(*shape_);
			break;
		case ColliderType::SPHERE:
			rigidActor_->detachShape(*shape_);
			SetSphereColliderData(rigidDynamicData.sphereColliderData);
			rigidActor_->attachShape(*shape_);
			break;
		case ColliderType::CAPSULE:
			rigidActor_->detachShape(*shape_);
			SetCapsuleColliderData(rigidDynamicData.capsuleColliderData);
			rigidActor_->attachShape(*shape_);
			break;
		default: break;
	}

	rigidActor_->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, true);
}

//-----------------------------------------------------------------------------
// RigidDynamicManager
//-----------------------------------------------------------------------------
RigidDynamicManager::RigidDynamicManager(EntityManager& entityManager,
	Transform3dManager& transform3dManager,
	PhysicsEngine& physicsEngine)
   : ComponentManager<RigidDynamic, EntityMask(ComponentType::RIGID_DYNAMIC)>(entityManager),
	 transform3dManager_(transform3dManager),
	 physicsEngine_(physicsEngine)
{}

void RigidDynamicManager::FixedUpdate(seconds dt)
{
	for (Entity entity = 0.0f; entity < entityManager_.get().GetEntitiesSize(); entity++)
	{
		if (!entityManager_.get().HasComponent(entity, EntityMask(ComponentType::RIGID_DYNAMIC)))
			continue;

		physx::PxTransform transform;
		transform = GetComponent(entity).GetPxRigidDynamic()->getGlobalPose();
		transform3dManager_.SetGlobalPosition(entity, ConvertFromPxVec(transform.p));

		transform3dManager_.SetGlobalRotation(
			entity, Quaternion::ToEulerAngles(ConvertFromPxQuat(transform.q)));
	}
}

void RigidDynamicManager::DestroyComponent(Entity entity)
{
	if (entity < components_.size())
	{
		if (GetComponent(entity).GetPxRigidDynamic())
			physicsEngine_.GetScene()->removeActor(*GetComponent(entity).GetPxRigidDynamic());
		SetComponent(entity, RigidDynamic());
	}

	ComponentManager::DestroyComponent(entity);
}

Entity RigidDynamicManager::FindEntityFromActor(physx::PxActor* actor)
{
	auto entityIt = std::find_if(components_.begin(),
		components_.end(),
		[actor](RigidDynamic rigidDynamic) { return actor == rigidDynamic.GetPxRigidDynamic(); });
	if (entityIt == components_.end()) return INVALID_ENTITY;

	return std::distance(components_.begin(), entityIt);
}

void RigidDynamicManager::AddRigidDynamic(Entity entity, const RigidDynamicData& rigidDynamicData)
{
	AddComponent(entity);
	const Vec3f position    = transform3dManager_.GetGlobalPosition(entity);
	const EulerAngles euler = transform3dManager_.GetGlobalRotation(entity);
	const Vec3f scale       = transform3dManager_.GetGlobalScale(entity);

	RigidDynamicData newRigidDynamicData = rigidDynamicData;
	newRigidDynamicData.boxColliderData.size =
		Vec3f(newRigidDynamicData.boxColliderData.size.x * scale.x,
			newRigidDynamicData.boxColliderData.size.y * scale.y,
			newRigidDynamicData.boxColliderData.size.z * scale.z);
	newRigidDynamicData.sphereColliderData.radius =
		newRigidDynamicData.sphereColliderData.radius * scale.x;
	newRigidDynamicData.capsuleColliderData.height =
		newRigidDynamicData.capsuleColliderData.height * scale.x;
	newRigidDynamicData.capsuleColliderData.radius =
		newRigidDynamicData.capsuleColliderData.radius * scale.z;

	RigidDynamic rigidDynamic = GetComponent(entity);
	rigidDynamic.Init(physicsEngine_, newRigidDynamicData, position, euler);
	physicsEngine_.GetScene()->addActor(*rigidDynamic.GetPxRigidDynamic());
	SetComponent(entity, rigidDynamic);
}

void RigidDynamicManager::AddForce(
	Entity entity, const Vec3f& force, physx::PxForceMode::Enum) const
{
	GetComponent(entity).AddForce(force);
}

void RigidDynamicManager::AddForceAtPosition(
	Entity entity, const Vec3f& force, const Vec3f& position) const
{
	GetComponent(entity).AddForceAtPosition(force, position);
}

void RigidDynamicManager::AddRelativeTorque(
	Entity entity, const Vec3f& torque, physx::PxForceMode::Enum forceMode) const
{
	GetComponent(entity).AddRelativeTorque(torque, forceMode);
}

RigidDynamicData RigidDynamicManager::GetRigidDynamicData(Entity entity) const
{
	return GetComponent(entity).GetRigidDynamicData();
}

VelocityData RigidDynamicManager::GetVelocityData(Entity entity) const
{
	return GetComponent(entity).GetVelocityData();
}

void RigidDynamicManager::SetPosition(Entity entity, const Vec3f& pos) const
{
	GetComponent(entity).SetPosition(pos);
}

void RigidDynamicManager::SetRotation(Entity entity, const Quaternion& rot) const
{
	GetComponent(entity).SetRotation(rot);
}

void RigidDynamicManager::SetLinearVelocity(Entity entity, const Vec3f& linearVelocity) const
{
	GetComponent(entity).GetPxRigidDynamic()->setLinearVelocity(ConvertToPxVec(linearVelocity));
}

void RigidDynamicManager::SetAngularVelocity(Entity entity, const Vec3f& angularVelocity) const
{
	GetComponent(entity).GetPxRigidDynamic()->setAngularVelocity(ConvertToPxVec(angularVelocity));
}

void RigidDynamicManager::SetRigidDynamicData(
	Entity entity, const RigidDynamicData& rigidDynamicData) const
{
	if (!physicsEngine_.IsPhysicRunning())
		GetComponent(entity).SetRigidDynamicData(rigidDynamicData);
}

//-----------------------------------------------------------------------------
// RigidDynamicSerializer
//-----------------------------------------------------------------------------
RigidDynamicSerializer::RigidDynamicSerializer(Transform3dManager& transform3dManager,
	EntityManager& entityManager,
	PhysicsEngine& physicsEngine,
	RigidDynamicManager& rigidDynamicManager)
   : ComponentSerializer(entityManager),
	 physicsEngine_(physicsEngine),
	 rigidDynamicManager_(rigidDynamicManager),
	 transform3dManager_(transform3dManager)
{}

void RigidDynamicSerializer::SetSelectedEntity(Entity selectedEntity)
{
	selectedEntity_ = selectedEntity;
	if (selectedEntity_ == INVALID_ENTITY) return;
	if (!physicsEngine_.IsPhysicRunning())
	{
		lastSelectedEntity_ = selectedEntity_;
		rigidDynamicData_   = rigidDynamicManager_.GetRigidDynamicData(lastSelectedEntity_);
		dynamicData_        = rigidDynamicManager_.GetVelocityData(lastSelectedEntity_);
		lastSelectedEntity_ = selectedEntity_;
	}
}

void RigidDynamicSerializer::FixedUpdate(seconds dt)
{
	lastSelectedEntity_ = selectedEntity_;
	if (lastSelectedEntity_ == INVALID_ENTITY) return;
	if (entityManager_.HasComponent(
			lastSelectedEntity_, static_cast<EntityMask>(ComponentType::RIGID_DYNAMIC)))
	{
		dynamicData_      = rigidDynamicManager_.GetVelocityData(lastSelectedEntity_);
		rigidDynamicData_ = rigidDynamicManager_.GetRigidDynamicData(lastSelectedEntity_);
	}
}

json RigidDynamicSerializer::GetJsonFromComponent(Entity entity) const
{
	json rigidDynamicViewer = json::object();
	if (entityManager_.HasComponent(entity, EntityMask(ComponentType::RIGID_DYNAMIC)))
	{
		if (entity != INVALID_ENTITY && entityManager_.GetEntitiesSize() > entity)
		{
			RigidDynamicData rigidDynamicData = rigidDynamicManager_.GetRigidDynamicData(entity);
			Vec3f scale                       = transform3dManager_.GetGlobalScale(entity);
			rigidDynamicData.boxColliderData.size =
				Vec3f(rigidDynamicData.boxColliderData.size.x / scale.x,
					rigidDynamicData.boxColliderData.size.y / scale.y,
					rigidDynamicData.boxColliderData.size.z / scale.z);
			rigidDynamicData.sphereColliderData.radius =
				rigidDynamicData.sphereColliderData.radius / scale.x;
			rigidDynamicData.capsuleColliderData.height =
				rigidDynamicData.capsuleColliderData.height / scale.x;
			rigidDynamicData.capsuleColliderData.radius =
				rigidDynamicData.capsuleColliderData.radius / scale.z;
			rigidDynamicViewer["useGravity"]        = rigidDynamicData.useGravity;
			rigidDynamicViewer["isKinematic"]       = rigidDynamicData.isKinematic;
			rigidDynamicViewer["isStatic"]          = false;
			rigidDynamicViewer["mass"]              = rigidDynamicData.mass;
			rigidDynamicViewer["linearDamping"]     = rigidDynamicData.linearDamping;
			rigidDynamicViewer["angularDamping"]    = rigidDynamicData.angularDamping;
			rigidDynamicViewer["rotationLock"]      = json::object();
			rigidDynamicViewer["rotationLock"]["x"] = rigidDynamicData.freezeRotation.x;
			rigidDynamicViewer["rotationLock"]["y"] = rigidDynamicData.freezeRotation.y;
			rigidDynamicViewer["rotationLock"]["z"] = rigidDynamicData.freezeRotation.z;
			rigidDynamicViewer["positionLock"]      = json::object();
			rigidDynamicViewer["positionLock"]["x"] = rigidDynamicData.freezePosition.x;
			rigidDynamicViewer["positionLock"]["y"] = rigidDynamicData.freezePosition.y;
			rigidDynamicViewer["positionLock"]["z"] = rigidDynamicData.freezePosition.z;
			rigidDynamicViewer["boxCollider"]       = GetJsonFromBoxCollider(rigidDynamicData);
			rigidDynamicViewer["sphereCollider"]    = GetJsonFromSphereCollider(rigidDynamicData);
			rigidDynamicViewer["capsuleCollider"]   = GetJsonFromCapsuleCollider(rigidDynamicData);
			rigidDynamicViewer["physicsMaterial"]   = GetJsonFromMaterial(rigidDynamicData);
		}
	}
	return rigidDynamicViewer;
}

void RigidDynamicSerializer::SetComponentFromJson(Entity entity, const json& componentJson)
{
	RigidDynamicData rigidDynamicData;
	rigidDynamicData.useGravity     = componentJson["useGravity"].get<bool>();
	rigidDynamicData.isKinematic    = componentJson["isKinematic"].get<bool>();
	rigidDynamicData.mass           = componentJson["mass"].get<float>();
	rigidDynamicData.linearDamping  = componentJson["linearDamping"].get<float>();
	rigidDynamicData.angularDamping = componentJson["angularDamping"].get<float>();

	rigidDynamicData.freezeRotation.x = componentJson["rotationLock"]["x"].get<bool>();
	rigidDynamicData.freezeRotation.y = componentJson["rotationLock"]["y"].get<bool>();
	rigidDynamicData.freezeRotation.z = componentJson["rotationLock"]["z"].get<bool>();

	rigidDynamicData.freezePosition.x = componentJson["positionLock"]["x"].get<bool>();
	rigidDynamicData.freezePosition.y = componentJson["positionLock"]["y"].get<bool>();
	rigidDynamicData.freezePosition.z = componentJson["positionLock"]["z"].get<bool>();

	RigidActorData rigidActorData        = SetFromJson(componentJson);
	rigidDynamicData.colliderType        = rigidActorData.colliderType;
	rigidDynamicData.boxColliderData     = rigidActorData.boxColliderData;
	rigidDynamicData.sphereColliderData  = rigidActorData.sphereColliderData;
	rigidDynamicData.capsuleColliderData = rigidActorData.capsuleColliderData;
	rigidDynamicData.material            = rigidActorData.material;

	std::string layer = aer::TagLocator::get().GetEntityLayer(entity);
	if (layer == "Ground") rigidDynamicData.filterGroup = FilterGroup::GROUND;
	else if (layer == "Ship")
		rigidDynamicData.filterGroup = FilterGroup::SHIP;
	else if (layer == "Wall")
		rigidDynamicData.filterGroup = FilterGroup::WALL;
	else
		rigidDynamicData.filterGroup = FilterGroup::DEFAULT;

	rigidDynamicManager_.AddRigidDynamic(entity, rigidDynamicData);
}

void RigidDynamicSerializer::DrawImGui(Entity entity)
{
	if (entity == INVALID_ENTITY) return;
	if (entityManager_.HasComponent(entity, EntityMask(ComponentType::RIGID_DYNAMIC)))
	{
		SetSelectedEntity(entity);
		if (lastSelectedEntity_ == INVALID_ENTITY) return;
		if (entityManager_.HasComponent(
				lastSelectedEntity_, EntityMask(ComponentType::RIGID_DYNAMIC)))
		{
			if (ImGui::TreeNode("Rigid Dynamic"))
			{
				RigidDynamicData rigidDynamicData = rigidDynamicData_;
				Vec3f linearVelocity              = dynamicData_.linearVelocity;
				ImGui::DragFloat3("Linear Velocity", linearVelocity.coord, 0);

				ImGui::DragFloat("Linear Damping", &rigidDynamicData.linearDamping);
				if (rigidDynamicData.linearDamping < 0) rigidDynamicData.linearDamping = 0;

				Vec3f angularVelocity = dynamicData_.angularVelocity;
				ImGui::DragFloat3("Angular Velocity", angularVelocity.coord, 0);

				ImGui::DragFloat("Angular Damping", &rigidDynamicData.angularDamping);
				if (rigidDynamicData.angularDamping < 0) rigidDynamicData.angularDamping = 0;

				ImGui::DragFloat("Mass", &rigidDynamicData.mass, 0.5f);
				if (rigidDynamicData.mass < 0) rigidDynamicData.mass = 0;

				ImGui::Checkbox("Use Gravity", &rigidDynamicData.useGravity);
				ImGui::Checkbox("Is Kinematic", &rigidDynamicData.isKinematic);
				ImGui::Text("Freeze Position");

				ImGui::SameLine();
				ImGui::Checkbox("X##freezePosition", &rigidDynamicData.freezePosition.x);

				ImGui::SameLine();
				ImGui::Checkbox("Y##freezePosition", &rigidDynamicData.freezePosition.y);

				ImGui::SameLine();
				ImGui::Checkbox("Z##freezePosition", &rigidDynamicData.freezePosition.z);

				ImGui::Text("Freeze Rotation");
				ImGui::SameLine();
				ImGui::Checkbox("X##freezeRotation", &rigidDynamicData.freezeRotation.x);

				ImGui::SameLine();
				ImGui::Checkbox("Y##freezeRotation", &rigidDynamicData.freezeRotation.y);

				ImGui::SameLine();
				ImGui::Checkbox("Z##freezeRotation", &rigidDynamicData.freezeRotation.z);

				RigidActorData rigidActorData        = DrawActorImGui(rigidDynamicData);
				rigidDynamicData.material            = rigidActorData.material;
				rigidDynamicData.boxColliderData     = rigidActorData.boxColliderData;
				rigidDynamicData.sphereColliderData  = rigidActorData.sphereColliderData;
				rigidDynamicData.capsuleColliderData = rigidActorData.capsuleColliderData;

				ImGui::TreePop();
				if (!physicsEngine_.IsPhysicRunning())
				{
					rigidDynamicManager_.SetRigidDynamicData(lastSelectedEntity_, rigidDynamicData);
					rigidDynamicData_ =
						rigidDynamicManager_.GetRigidDynamicData(lastSelectedEntity_);
					dynamicData_        = rigidDynamicManager_.GetVelocityData(lastSelectedEntity_);
					lastSelectedEntity_ = selectedEntity_;
				}
			}
		}
	}
}
}    // namespace neko::physics
