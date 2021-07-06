#include "vk/material/material_manager.h"

namespace neko::vk
{
MaterialManager::MaterialManager() { MaterialManagerLocator::provide(this); }

void MaterialManager::Clear()
{
	diffuseMaterials_.clear();

	//skyboxMaterialIds_.clear();
	//skyboxMaterials_.clear();

	//trailMaterialIds_.clear();
	//trailMaterials_.clear();

	//particleMaterials_.clear();
	//particleMaterialIds_.clear();

	diffuseMaterials_.emplace(kDefaultMaterialId, DiffuseMaterial());
	diffuseMaterials_[kDefaultMaterialId].CreatePipeline(false);
}

MaterialId MaterialManager::AddMaterial(std::string_view materialPath)
{
	const json materialJson         = LoadJson(materialPath);
	const MaterialType materialType = materialJson["type"];
	const MaterialId resourceId     = sole::uuid0();
	switch (materialType)
	{
		case MaterialType::DIFFUSE:
		{
			auto& textureManager = TextureManagerLocator::get();
			diffuseMaterials_.emplace(resourceId, DiffuseMaterial());

			// Textures defined in the material's JSON use the relative path to the data folder
			// defined in "BasicEngine::config->dataRootPath"
			if (CheckJsonExists(materialJson, "diffusePath"))
				textureManager.AddTexture(materialJson["diffusePath"].get<std::string_view>());

			if (CheckJsonExists(materialJson, "specularPath"))
				textureManager.AddTexture(materialJson["specularPath"].get<std::string_view>());

			if (CheckJsonExists(materialJson, "normalPath"))
				textureManager.AddTexture(materialJson["normalPath"].get<std::string_view>());

			diffuseMaterials_[resourceId].FromJson(materialJson);
			diffuseMaterials_[resourceId].CreatePipeline(false);
			break;
		}
	}

	return resourceId;
}

MaterialId MaterialManager::AddNewMaterial(MaterialType materialType, MaterialId id)
{
	if (id == MaterialId()) id = sole::uuid0();
	switch (materialType)
	{
		case MaterialType::DIFFUSE:
		{
			if (diffuseMaterials_.find(id) != diffuseMaterials_.cend()) return id;
			diffuseMaterials_.emplace(id, DiffuseMaterial());
			break;
		}
	}

	return id;
}

Material& MaterialManager::GetMaterial(const MaterialId resourceId)
{
	if (resourceId == kDefaultMaterialId &&
		diffuseMaterials_.find(resourceId) == diffuseMaterials_.cend())
	{
		diffuseMaterials_.emplace(kDefaultMaterialId, DiffuseMaterial());
		diffuseMaterials_[kDefaultMaterialId].CreatePipeline(false);
	}

	//Diffuse materials
	if (diffuseMaterials_.find(resourceId) != diffuseMaterials_.cend())
		return diffuseMaterials_[resourceId];

	//Particles materials
	//for (size_t i = 0; i < particleMaterialIDs_.size(); i++)
	//	if (particleMaterialIDs_[i] == resourceID) return particleMaterials_[i];

	//Trails materials
	//for (size_t i = 0; i < trailMaterialIDs_.size(); i++)
	//	if (trailMaterialIDs_[i] == resourceID) return trailMaterials_[i];

	//Skybox materials
	//for (size_t i = 0; i < skyboxMaterialIDs_.size(); i++)
	//	if (skyboxMaterialIDs_[i] == resourceID) return skyboxMaterials_[i];

	return diffuseMaterials_[kDefaultMaterialId];
}

DiffuseMaterial& MaterialManager::GetDiffuseMaterial(MaterialId resourceId)
{
	if (resourceId == kDefaultMaterialId &&
	    diffuseMaterials_.find(resourceId) == diffuseMaterials_.cend())
	{
		diffuseMaterials_.emplace(kDefaultMaterialId, DiffuseMaterial());
		diffuseMaterials_[kDefaultMaterialId].CreatePipeline(false);
	}

	return diffuseMaterials_[resourceId];
}

bool MaterialManager::IsMaterialLoaded(MaterialId resourceId)
{
	if (diffuseMaterials_.find(resourceId) != diffuseMaterials_.cend()) return true;

	return false;
}
}    // namespace neko::vk