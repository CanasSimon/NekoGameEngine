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
#include "PxPhysicsAPI.h"

#include "engine/entity.h"
#include "engine/transform.h"

#include "px/filter_group.h"
#include "px/raycast.h"
#include "px/rigidbody.h"

namespace neko::physics
{
/// Wrapper around PhysX elements
class PhysicsEngine : public neko::SystemInterface
{
public:
    void Init() override {}
    void InitPhysics();
    void Update(seconds dt) override;
    void Destroy() override;

	/// Launch a raycast through the scene
	/// \param origin Origin position of the ray
	/// \param direction Vector of direction of the ray
	/// \param maxDistance MaxDistance of the ray
	/// \return the RaycastInfo with all raycast infos
	[[nodiscard]] RaycastInfo Raycast(const Vec3f& origin,
		const Vec3f& direction,
		float maxDistance,
		FilterGroup::Enum filterGroup = FilterGroup::EVERYTHING) const;

	/// Register an object for the OnCollision event
    void RegisterCollisionListener(OnCollisionInterface& collisionInterface);

    /// Register an object for the OnTrigger event
    void RegisterTriggerListener(OnTriggerInterface& triggerInterface);

    /// Register an object for the FixedUpdate event
    void RegisterFixedUpdateListener(FixedUpdateInterface& fixedUpdateInterface);

    /// Start the physics simulation
    void StartPhysic() { physicRunning_ = true; }

    /// Stop the physics simulation
    void StopPhysic() { physicRunning_ = false; }

    [[nodiscard]] physx::PxPhysics* GetPhysx() const { return physics_; }
    [[nodiscard]] physx::PxScene* GetScene() const { return scene_; }
    [[nodiscard]] physx::PxCooking* GetCooking() const { return cooking_; }

    /// \return if the physics simulation is running
    [[nodiscard]] bool IsPhysicRunning() const { return physicRunning_; }

private:
	/// Create the PxScene and set scene parameters
	void CreateScene();

	/// Check if the fixed step is done
	bool Advance(physx::PxReal dt);

	/// Simulate the physic and call FixedUpdate Listener
	void FixedUpdate(seconds dt);

	static physx::PxFilterFlags ContactReportFilterShader(
		physx::PxFilterObjectAttributes attributes0,
		physx::PxFilterData,
		physx::PxFilterObjectAttributes attributes1,
		physx::PxFilterData,
		physx::PxPairFlags& pairFlags,
		const void*,
		physx::PxU32);

	physx::PxFoundation* foundation_              = nullptr;
	physx::PxPhysics* physics_                    = nullptr;
	physx::PxPvd* pvd_                            = nullptr;
	physx::PxPvdTransport* transport_             = nullptr;
	physx::PxCooking* cooking_                    = nullptr;
	physx::PxDefaultCpuDispatcher* cpuDispatcher_ = nullptr;
	physx::PxScene* scene_                        = nullptr;

	float accumulator_ = 0.0f;
    seconds stepSize_ = seconds(0.02f); // FixedStep duration

    bool physicRunning_ = false;

    PhysicsSimulationEventCallback eventCallback_;

	Action<seconds> fixedUpdateAction_;
};
}    // namespace neko::physics
