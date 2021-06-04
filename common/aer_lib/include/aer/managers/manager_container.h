#pragma once
#include "engine/transform.h"

#ifdef NEKO_GLES3
#include "gl/model.h"
#include "gl/texture.h"
#elif NEKO_VULKAN
#include "vk/material/material_manager.h"
#endif

#ifdef NEKO_FMOD
#include "fmod/audio_manager.h"
#endif

#include "px/physics_engine.h"
#include "px/rigidbody.h"

#include "aer/managers/light_manager.h"
#include "aer/managers/render_manager.h"
#include "aer/scene.h"

namespace neko::aer
{
struct ResourceManagerContainer : public SystemInterface
{
	void Init() override
	{
		textureManager.Init();
		modelManager.Init();
	}

	void Update(seconds dt) override
	{
		textureManager.Update(dt);
		modelManager.Update(dt);
	}

	void Destroy() override
	{
		textureManager.Destroy();
		modelManager.Destroy();
	}

#ifdef NEKO_GLES3
	gl::TextureManager textureManager;
	gl::ModelManager modelManager;
#else
	vk::TextureManager textureManager;
	vk::ModelManager modelManager;
	vk::MaterialManager materialManager;
#endif
};

struct ComponentManagerContainer : public SystemInterface
{
	ComponentManagerContainer(AerEngine& engine,
		ResourceManagerContainer& rContainer,
		physics::PhysicsEngine& physicsEngine)
	   : transform3dManager(entityManager),
		 sceneManager(entityManager, *this),
		 renderManager(entityManager, rContainer.modelManager, transform3dManager, lightManager),
		 lightManager(entityManager, transform3dManager),
		 rigidDynamicManager(entityManager, transform3dManager, physicsEngine),
		 rigidStaticManager(entityManager, transform3dManager, renderManager, physicsEngine),
#ifdef NEKO_FMOD
		 audioManager(entityManager, transform3dManager),
#endif
		 transform3dSerializer(entityManager, transform3dManager),
		 rendererSerializer(entityManager, renderManager),
		 lightSerializer(entityManager, lightManager),
		 rigidDynamicSerializer(transform3dManager, entityManager, physicsEngine, rigidDynamicManager),
		 rigidStaticSerializer(transform3dManager, entityManager, physicsEngine, rigidStaticManager)
#ifdef NEKO_FMOD
		 ,
		 audioSerializer(entityManager, audioManager)
#endif
	{
		physicsEngine.RegisterFixedUpdateListener(rigidDynamicManager);
		physicsEngine.RegisterFixedUpdateListener(rigidStaticManager);
		physicsEngine.RegisterFixedUpdateListener(rigidStaticSerializer);
		physicsEngine.RegisterFixedUpdateListener(rigidDynamicSerializer);
	}

	void Init() override
	{
		transform3dManager.Init();
		renderManager.Init();
#ifdef NEKO_FMOD
		audioManager.Init();
#endif
	}

	void Update(seconds dt) override
	{
		transform3dManager.Update();
		renderManager.Update(dt);
#ifdef NEKO_FMOD
		audioManager.Update(dt);
#endif
		transform3dManager.Update();
	}

	void Destroy() override
	{
		renderManager.Destroy();
#ifdef NEKO_FMOD
		audioManager.Destroy();
#endif
	}

	EntityManager entityManager;
	Transform3dManager transform3dManager;
	SceneManager sceneManager;
	RenderManager renderManager;
	LightManager lightManager;
	physics::RigidDynamicManager rigidDynamicManager;
	physics::RigidStaticManager rigidStaticManager;
#ifdef NEKO_FMOD
	AudioManager audioManager;
#endif

	Transform3dSerializer transform3dSerializer;
	RendererSerializer rendererSerializer;
	LightSerializer lightSerializer;
	physics::RigidDynamicSerializer rigidDynamicSerializer;
	physics::RigidStaticSerializer rigidStaticSerializer;
#ifdef NEKO_FMOD
	AudioSerializer audioSerializer;
#endif
};
}    // namespace neko::aer
