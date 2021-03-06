#include "vk/material/material_manager.h"
#include "vk/vk_resources.h"

namespace neko::vk
{
ModelManager::ModelManager() { ModelManagerLocator::provide(this); }

void ModelManager::Init() {}

void ModelManager::Update(seconds)
{
	while (!loaders_.empty())
	{
		auto& modelLoader = loaders_.front();
		modelLoader.Update();
		if (modelLoader.HasErrors()) { loaders_.pop(); }
		else if (modelLoader.IsDone())
		{
			const ModelId modelId = modelLoader.GetModelId();
			models_[modelId]      = *modelLoader.GetModel();
			loaders_.pop();
		}
		else
		{
			break;
		}
	}
}

void ModelManager::Destroy()
{
	VkResources::Inst->GetCurrentCmdBuffer().SubmitIdle(false);

	for (auto& model : models_) model.second.Destroy();
}

ModelId ModelManager::LoadModel(std::string_view path)
{
	const auto it = pathMap_.find(path.data());
	if (it != pathMap_.end())
	{
		return it->second;
	}

	LogDebug(fmt::format("Loading model: {}", path));
	const std::string metaPath  = fmt::format("{}.meta", path);
	const json metaJson         = LoadJson(metaPath);
	ModelId modelId             = INVALID_MODEL_ID;
	if (CheckJsonExists(metaJson, "uuid"))
	{
		modelId = sole::rebuild(metaJson["uuid"].get<std::string>());
	}
	else
	{
		LogError(fmt::format("Could not find model id in json file: {}", metaPath));
		return modelId;
	}

	pathMap_.emplace(path.data(), modelId);
	loaders_.push(ModelLoader(path, modelId));
	loaders_.back().Start();
	return modelId;
}

const Model* ModelManager::GetModel(ModelId modelId) const
{
	if (modelId == INVALID_MODEL_ID) return nullptr;

	const auto it = models_.find(modelId);
	if (it != models_.end()) return &it->second;

	return nullptr;
}

std::string ModelManager::GetModelName(ModelId modelId)
{
	for (auto& it : pathMap_)
	{
		if (it.second == modelId)
		{
			std::size_t startName         = it.first.find_last_of('/') + 1;
			std::size_t endName           = it.first.find_first_of('.');
			std::size_t size              = endName - startName;
			std::string name              = it.first.substr(startName, size);
			return name;
		}
	}

	return "";
}

std::string_view ModelManager::GetModelPath(ModelId modelId)
{
	for (auto& it : pathMap_)
		if (it.second == modelId)
			return it.first;

	return "";
}

bool ModelManager::IsLoaded(ModelId modelId) const
{
	const auto* model = GetModel(modelId);
	if (!model) return false;

	auto& materialManager = MaterialManagerLocator::get();
	for (const auto& mesh : model->GetMeshes())
		if (!materialManager.IsMaterialLoaded(mesh.GetMaterialId())) return false;

	return true;
}
}    // namespace neko::vk