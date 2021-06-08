#include "gl/model_manager.h"

namespace neko::gl
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
			models_[modelLoader.GetModelId()] = *modelLoader.GetModel();
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
	for (auto& model : models_) model.second.Destroy();
}

const Model* ModelManager::GetModel(ModelId modelId) const
{
	if (modelId == INVALID_MODEL_ID) return nullptr;

	const auto it = models_.find(modelId);
	if (it != models_.end()) return &it->second;
	return nullptr;
}

Model* ModelManager::GetModelPtr(ModelId modelId)
{
	if (modelId == INVALID_MODEL_ID) return nullptr;

	const auto it = models_.find(modelId);
	if (it != models_.end()) return &models_[modelId];
	return nullptr;
}

ModelId ModelManager::LoadModel(std::string_view path)
{
	const auto it = pathMap_.find(path.data());
	if (it != pathMap_.end()) { return it->second; }

	const std::string metaPath = fmt::format("{}.meta", path);
	auto metaJson              = LoadJson(metaPath);
	ModelId modelId            = INVALID_MODEL_ID;
	if (CheckJsonExists(metaJson, "uuid"))
	{
		modelId = sole::rebuild(metaJson["uuid"].get<std::string>());
	}
	else
	{
        LogError(fmt::format("Could not find model id in json file: {}", metaPath));
		return modelId;
	}

	loaders_.push(ModelLoader(path, modelId));
	loaders_.back().Start();
	pathMap_.emplace(path, modelId);
	return modelId;
}

std::string ModelManager::GetModelName(ModelId modelId)
{
	for (auto& it : pathMap_)
	{
		if (it.second == modelId)
		{
			std::size_t startName = it.first.find_last_of('/') + 1;
			std::size_t endName   = it.first.find_first_of('.');
			std::size_t size      = endName - startName;
			std::string name      = it.first.substr(startName, size);
			return name;
		}
	}

	return "";
}

std::string_view ModelManager::GetModelPath(ModelId modelId)
{
	for (auto& it : pathMap_)
		if (it.second == modelId) return it.first;

	return "";
}
}    // namespace neko::gl