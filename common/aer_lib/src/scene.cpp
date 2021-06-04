#include "engine/engine.h"
#include "engine/resource_locations.h"
#include "utils/file_utility.h"

#include "aer/log.h"
#include "aer/managers/manager_container.h"

#ifdef NEKO_VULKAN
#include "vk/vk_resources.h"
#endif

#ifdef EASY_PROFILE_USE
#include "easy/profiler.h"
#endif

namespace neko::aer
{
SceneManager::SceneManager(
	EntityManager& entityManager, ComponentManagerContainer& componentManagerContainer)
   : filesystem_(BasicEngine::GetInstance()->GetFilesystem()),
	 entityManager_(entityManager),
	 tagManager_(*this),
	 componentManagerContainer_(componentManagerContainer)
{
	TagLocator::provide(&tagManager_);
}

void SceneManager::ParseComponentJson(const json& componentJson, Entity entity)
{
	// Transform
	if (CheckJsonParameter(componentJson, "transform", json::value_t::object))
	{
		if (CheckJsonParameter(componentJson["transform"], "exist", json::value_t::boolean))
		{
			if (componentJson["transform"]["exist"])
			{
				componentManagerContainer_.transform3dManager.AddComponent(entity);
				componentManagerContainer_.transform3dSerializer.SetComponentFromJson(
					entity, componentJson["transform"]);
			}
		}
	}

	// Rigidbody
	if (CheckJsonParameter(componentJson, "rigidbody", json::value_t::object))
	{
		if (CheckJsonParameter(componentJson["rigidbody"], "exist", json::value_t::boolean))
		{
			if (componentJson["rigidbody"]["exist"])
			{
				if (CheckJsonParameter(
						componentJson["rigidbody"], "isStatic", json::value_t::boolean))
				{
					if (componentJson["rigidbody"]["isStatic"])
					{
						componentManagerContainer_.rigidStaticSerializer.SetComponentFromJson(
							entity, componentJson["rigidbody"]);
					}
					else
					{
						componentManagerContainer_.rigidDynamicSerializer.SetComponentFromJson(
							entity, componentJson["rigidbody"]);
					}
				}
			}
		}
	}

	// Model
	if (CheckJsonParameter(componentJson, "modelRenderer", json::value_t::object))
	{
		if (CheckJsonParameter(componentJson["modelRenderer"], "exist", json::value_t::boolean))
		{
			if (componentJson["modelRenderer"]["exist"])
			{
				componentManagerContainer_.renderManager.AddComponent(entity);
				componentManagerContainer_.rendererSerializer.SetComponentFromJson(
					entity, componentJson["modelRenderer"]);
			}
		}
	}

	// MeshCollider
	if (CheckJsonParameter(componentJson, "meshCollider", json::value_t::object))
	{
		if (CheckJsonParameter(componentJson["meshCollider"], "exist", json::value_t::boolean))
		{
			if (componentJson["meshCollider"]["exist"])
			{
				componentManagerContainer_.rigidStaticManager.AddMeshColliderStatic(entity,
					componentJson["meshCollider"]["meshColliderName"].get<std::string_view>());
			}
		}
	}

	// Light
	if (CheckJsonParameter(componentJson, "light", json::value_t::object))
	{
		if (CheckJsonParameter(componentJson["light"], "exist", json::value_t::boolean))
		{
			if (componentJson["light"]["exist"])
			{
				componentManagerContainer_.lightManager.AddComponent(entity);
				componentManagerContainer_.lightSerializer.SetComponentFromJson(
					entity, componentJson["light"]);
			}
		}
	}

#ifdef NEKO_FMOD
	// Audio Source
	if (CheckJsonParameter(componentJson, "audioSource", json::value_t::object))
	{
		if (CheckJsonParameter(componentJson["audioSource"], "exist", json::value_t::boolean))
		{
			if (componentJson["audioSource"]["exist"])
			{
				componentManagerContainer_.audioManager.AddComponent(entity);
				componentManagerContainer_.audioSerializer.SetComponentFromJson(
					entity, componentJson["audioSource"]);
			}
		}
	}
#endif
}

void SceneManager::ParseEntityJson(const json& entityJson)
{
    // Entity values
    Entity entity = entityManager_.CreateEntity(entityJson["instanceId"].get<int>());
	if (CheckJsonParameter(entityJson, "isActive", json::value_t::boolean))
	{
		if (!entityJson["isActive"]) return;
	}

	if (CheckJsonParameter(entityJson, "name", json::value_t::string))
	{
		entityManager_.SetEntityName(entity, entityJson["name"].get<std::string_view>());
	}

    // Scene values
	if (CheckJsonParameter(entityJson, "layer", json::value_t::string))
	{
		TagLocator::get().SetEntityLayer(entity, static_cast<std::string>(entityJson["layer"]));
	}

	if (CheckJsonParameter(entityJson, "tag", json::value_t::string))
	{
		TagLocator::get().SetEntityTag(entity, static_cast<std::string>(entityJson["tag"]));
	}

	//TODO Set active
	ParseComponentJson(entityJson, entity);
}

void SceneManager::ParseSceneJson(const json& sceneJson)
{
	// Scene information
	if (CheckJsonParameter(sceneJson, "name", json::value_t::string))
		currentScene_.sceneName = sceneJson["name"];
	else
		currentScene_.sceneName = "New Scene";

	if (CheckJsonParameter(sceneJson, "tags", json::value_t::array))
		for (auto& tag : sceneJson["tags"])
			if (!TagExist(tag)) AddTag(tag);

	if (CheckJsonParameter(sceneJson, "layers", json::value_t::array))
		for (auto& layer : sceneJson["layers"])
			if (!LayerExist(layer)) AddLayer(layer);

	// Directional Light
	if (CheckJsonParameter(sceneJson, "dirLight", json::value_t::object))
		DirectionalLight::Instance->FromJson(sceneJson["dirLight"]);

	// Components
	if (CheckJsonParameter(sceneJson, "objects", json::value_t::array))
	{
		for (auto&& object : sceneJson["objects"])
		{
			const auto entity           = object["instanceId"].get<InstanceId>();
			const auto parentEntity     = object["parent"].get<InstanceId>();
			if (parentEntity != INVALID_INSTANCE_ID)
				entityManager_.SetParent(entity, parentEntity);

            ParseEntityJson(object);
		}
	}
}

bool SceneManager::LoadScene(const std::string_view& jsonPath)
{
#ifdef EASY_PROFILE_USE
	EASY_BLOCK("LoadScene");
#endif
	if (filesystem_.FileExists(jsonPath))
	{
		json scene = neko::LoadJson(jsonPath);
		if (!scene.is_object())
		{
			LogError(fmt::format("Scene reading failed {}", jsonPath));
			return false;
		}
		currentScene_.scenePath = jsonPath;
#ifdef NEKO_VULKAN
		auto& cmdBuffers = vk::VkResources::Inst->modelCommandBuffers;
		for (std::size_t i = 0; i < vk::VkResources::Inst->GetViewportCount(); ++i)
			cmdBuffers[i].Destroy();
#endif
		entityManager_.Clear();
		ParseSceneJson(scene);
		return true;
	}
	else
	{
		LogError(fmt::format("Scene not found {}", jsonPath));
		return false;
	}
}

void SceneManager::SaveCurrentScene()
{
	WriteStringToFile(
		GetScenesFolderPath() + currentScene_.sceneName + ".aerscene", WriteSceneJson().dump(4));
	currentScene_.saved = true;
}

json SceneManager::WriteEntityJson(Entity entity) const
{
	// Entity information
	json entityJson          = json::object();
	entityJson["name"]       = entityManager_.GetEntityName(entity);
	entityJson["tag"]        = TagLocator::get().GetEntityTag(entity);
	entityJson["instanceId"] = entity;
	entityJson["parent"]     = entityManager_.GetParent(entity);
	entityJson["layer"]      = TagLocator::get().GetEntityLayer(entity);
	//entityJson["isActive"] = TagLocator::get().GetEntityTag(entity);  //TODO Set active

	// Transform
	entityJson["transform"] = json::object();
	entityJson["transform"] =
		componentManagerContainer_.transform3dSerializer.GetJsonFromComponent(entity);
	entityJson["transform"]["exist"] =
		entityManager_.HasComponent(entity, EntityMask(ComponentType::TRANSFORM3D));

	// Rigidbodies
	entityJson["rigidbody"] = json::object();
	if (entityManager_.HasComponent(entity, EntityMask(ComponentType::RIGID_STATIC)))
		entityJson["rigidbody"] =
			componentManagerContainer_.rigidStaticSerializer.GetJsonFromComponent(entity);
	else if (entityManager_.HasComponent(entity, EntityMask(ComponentType::RIGID_DYNAMIC)))
		entityJson["rigidbody"] =
			componentManagerContainer_.rigidDynamicSerializer.GetJsonFromComponent(entity);

	entityJson["rigidbody"]["exist"] =
		entityManager_.HasComponent(entity, EntityMask(ComponentType::RIGID_DYNAMIC)) ||
		entityManager_.HasComponent(entity, EntityMask(ComponentType::RIGID_STATIC));

	// Models
	entityJson["modelRenderer"] = json::object();
	entityJson["modelRenderer"] =
		componentManagerContainer_.rendererSerializer.GetJsonFromComponent(entity);
	entityJson["modelRenderer"]["exist"] =
		entityManager_.HasComponent(entity, EntityMask(ComponentType::MODEL));

	// Lights
	entityJson["light"] = json::object();
	entityJson["light"] = componentManagerContainer_.lightSerializer.GetJsonFromComponent(entity);
	entityJson["light"]["exist"] =
		entityManager_.HasComponent(entity, EntityMask(ComponentType::LIGHT));

#ifdef NEKO_FMOD
	// Audio Sources
	entityJson["audioSource"] = json::object();
	entityJson["audioSource"] = componentManagerContainer_.audioSerializer.GetJsonFromComponent(entity);
	entityJson["audioSource"]["exist"] =
		entityManager_.HasComponent(entity, EntityMask(ComponentType::AUDIO_SOURCE));
#endif

	return entityJson;
}

json SceneManager::WriteSceneJson()
{
	// Scene data
	json scene      = json::object();
	scene["name"]   = currentScene_.sceneName;
	scene["tags"]   = currentScene_.tags;
	scene["layers"] = currentScene_.layers;

	// Directional Light
	scene["dirLight"] = DirectionalLight::Instance->ToJson();

	// Components
	scene["objects"] = json::array_t();
	for (size_t i = 0; i < entityManager_.GetEntitiesNmb(); ++i)
		scene["objects"][i] = WriteEntityJson(i);

	return scene;
}

void SceneManager::AddTag(const std::string& newTagName)
{
	if (!LayerExist(newTagName)) currentScene_.tags.push_back(newTagName);
	else LogDebug("Tag already set");
}

void SceneManager::AddLayer(const std::string& newLayerName)
{
	if (!LayerExist(newLayerName)) currentScene_.layers.push_back(newLayerName);
	else LogDebug("Layer already set");
}

bool SceneManager::TagExist(const std::string& newTagName)
{
	const auto entityTagIt = std::find_if(currentScene_.tags.begin(),
		currentScene_.tags.end(),
		[newTagName](std::string_view tagName) { return newTagName == tagName; });
	if (entityTagIt == currentScene_.tags.end()) { return false; }
	return true;
}

bool SceneManager::LayerExist(const std::string& newLayerName)
{
	const auto entityLayerIt = std::find_if(currentScene_.layers.begin(),
		currentScene_.layers.end(),
		[newLayerName](std::string_view layerName) { return newLayerName == layerName; });
	if (entityLayerIt == currentScene_.layers.end()) { return false; }
	return true;
}

const std::vector<std::string>& SceneManager::GetTags() const { return currentScene_.tags; }

const std::vector<std::string>& SceneManager::GetLayers() const { return currentScene_.layers; }
}    // namespace neko::aer
