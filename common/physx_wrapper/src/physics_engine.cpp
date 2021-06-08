#include "px/physics_engine.h"

#include "engine/log.h"
#include "engine/engine.h"

#include "px/physx_utility.h"

namespace neko::physics
{
void PhysicsEngine::InitPhysics()
{
	static physx::PxDefaultErrorCallback gDefaultErrorCallback;
	static physx::PxDefaultAllocator gDefaultAllocatorCallback;

	foundation_ =
		PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
	if (!foundation_)
	{
		assert("PxCreateFoundation failed!");
		return;
	}

	//Use to link with PhysX Visual Debugger
	bool recordMemoryAllocations = true;
	std::string PVD_HOST         = "localhost";
	pvd_                         = physx::PxCreatePvd(*foundation_);
	transport_ = physx::PxDefaultPvdSocketTransportCreate(PVD_HOST.c_str(), 5425, 10);

	bool connected = pvd_->connect(*transport_, physx::PxPvdInstrumentationFlag::eALL);
	if (connected) logDebug("Connected to PhysX Visual Debugger!");

	physics_ = PxCreatePhysics(PX_PHYSICS_VERSION,
		*foundation_,
		physx::PxTolerancesScale(),
		recordMemoryAllocations,
		pvd_);
	neko_assert(physics_, "PxCreatePhysics failed!");

	cooking_ = PxCreateCooking(
		PX_PHYSICS_VERSION, *foundation_, physx::PxCookingParams(physx::PxTolerancesScale()));
	neko_assert(cooking_, "PxCreateCooking failed!");
	neko_assert(PxInitExtensions(*physics_, pvd_), "PxInitExtensions failed!");

	CreateScene();
}

void PhysicsEngine::CreateScene()
{
	cpuDispatcher_ = physx::PxDefaultCpuDispatcherCreate(1);
	neko_assert(cpuDispatcher_, "PxDefaultCpuDispatcherCreate failed!");

	physx::PxSceneDesc sceneDesc      = physx::PxTolerancesScale();
	sceneDesc.gravity                 = physx::PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.filterShader            = ContactReportFilterShader;
	sceneDesc.cpuDispatcher           = cpuDispatcher_;
	sceneDesc.kineKineFilteringMode   = physx::PxPairFilteringMode::eKEEP; // So kin-kin contacts can be reported
	sceneDesc.staticKineFilteringMode = physx::PxPairFilteringMode::eKEEP; // So static-kin contacts can be reported
	sceneDesc.simulationEventCallback = &eventCallback_;
	sceneDesc.flags |= physx::PxSceneFlag::eENABLE_CCD; //Use when Continuous Detection

	scene_ = physics_->createScene(sceneDesc);
	if (transport_->isConnected())
	{
		neko_assert(scene_, "createScene failed!");
		scene_->getScenePvdClient()->setScenePvdFlag(
			physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		scene_->getScenePvdClient()->setScenePvdFlag(
			physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		scene_->getScenePvdClient()->setScenePvdFlag(
			physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
}

bool PhysicsEngine::Advance(physx::PxReal dt)
{
    accumulator_ += dt;
    if (accumulator_ < stepSize_.count())
        return false;

    return true;
}

void PhysicsEngine::Update(seconds dt)
{
#ifdef EASY_PROFILE_USE
    EASY_BLOCK("PhysicsEngine::Update");
#endif
	if (physicRunning_)
	{
		if (Advance(dt.count()))
		{
			if (accumulator_ > 1.0f)
			{
				while (accumulator_ > 1.0f)
				{
					accumulator_ -= stepSize_.count();
					FixedUpdate(stepSize_);
				}
			}
			else
			{
				accumulator_ -= stepSize_.count();
				FixedUpdate(stepSize_);
			}
		}
	}
}

void PhysicsEngine::FixedUpdate(seconds dt)
{
    //scene_->simulate(stepSize_.count());
    scene_->collide(dt.count());
    scene_->fetchCollision(true);
    scene_->advance();    // Can this be skipped
    scene_->fetchResults(true);
    fixedUpdateAction_.Execute(dt);
}

void PhysicsEngine::Destroy()
{
    pvd_->disconnect();
    physics_->release();
    cooking_->release();
    PxCloseExtensions();
    pvd_->release();
    transport_->release();
    foundation_->release();
}

physx::PxFilterFlags PhysicsEngine::ContactReportFilterShader(
	physx::PxFilterObjectAttributes attributes0,
	physx::PxFilterData,
	physx::PxFilterObjectAttributes attributes1,
	physx::PxFilterData,
	physx::PxPairFlags& pairFlags,
	const void*,
	physx::PxU32)
{
	// let triggers through
	if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
	{
		pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
		return physx::PxFilterFlag::eDEFAULT;
	}
	{
		pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;
		pairFlags |= physx::PxPairFlag::eDETECT_CCD_CONTACT;
		pairFlags |=
			physx::PxPairFlag::eNOTIFY_TOUCH_FOUND | physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS |
			physx::PxPairFlag::eNOTIFY_TOUCH_LOST | physx::PxPairFlag::eNOTIFY_CONTACT_POINTS;
	}

	return physx::PxFilterFlag::eDEFAULT;
}

RaycastInfo PhysicsEngine::Raycast(const Vec3f& origin,
	const Vec3f& direction,
	float maxDistance,
	FilterGroup::Enum filterGroup) const
{
	RaycastInfo raycastHit;
	physx::PxQueryFilterData fd;
	fd.data.word0    = filterGroup;
	raycastHit.touch = scene_->raycast(ConvertToPxVec(origin),
		ConvertToPxVec(direction.Normalized()),
		maxDistance,
		raycastHit.pxRaycastBuffer,
		physx::PxHitFlags(physx::PxHitFlag::eDEFAULT),
		fd);

	return raycastHit;
}

void PhysicsEngine::RegisterCollisionListener(OnCollisionInterface& collisionInterface)
{
	eventCallback_.onCollisionEnterAction.RegisterCallback(
		[&collisionInterface](const physx::PxContactPairHeader& pairHeader)
		{ collisionInterface.OnCollisionEnter(pairHeader); });
	eventCallback_.onCollisionStayAction.RegisterCallback(
		[&collisionInterface](const physx::PxContactPairHeader& pairHeader)
		{ collisionInterface.OnCollisionStay(pairHeader); });
	eventCallback_.onCollisionExitAction.RegisterCallback(
		[&collisionInterface](const physx::PxContactPairHeader& pairHeader)
		{ collisionInterface.OnCollisionExit(pairHeader); });
}

void PhysicsEngine::RegisterTriggerListener(OnTriggerInterface& triggerInterface)
{
	eventCallback_.onTriggerEnterAction.RegisterCallback(
		[&triggerInterface](physx::PxTriggerPair* pairs)
		{ triggerInterface.OnTriggerEnter(pairs); });
	eventCallback_.onTriggerExitAction.RegisterCallback(
		[&triggerInterface](physx::PxTriggerPair* pairs)
		{ triggerInterface.OnTriggerExit(pairs); });
}

void PhysicsEngine::RegisterFixedUpdateListener(FixedUpdateInterface& fixedUpdateInterface)
{
	fixedUpdateAction_.RegisterCallback(
		[&fixedUpdateInterface](seconds dt) { fixedUpdateInterface.FixedUpdate(dt); });
}
}    // namespace neko::physics
