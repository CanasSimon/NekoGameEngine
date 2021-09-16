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

 Author: Canas Simon
 Date:
---------------------------------------------------------- */
#include <sole.hpp>

#include "vk/images/texture_manager.h"
#include "vk/material/diffuse_material.h"
#include "vk/material/particle_material.h"

namespace neko::vk
{
constexpr size_t kMaterialDefaultNum = 100;
//constexpr size_t kMaterialsSkyboxDefaultNum           = 2;
static MaterialId kDefaultMaterialId = sole::uuid0();
//static MaterialId kDefaultSkyboxMaterialId         = HashString(kDefaultSkyboxMaterialName);

class IMaterialManager
{
public:
	virtual ~IMaterialManager() = default;

	[[maybe_unused]] virtual MaterialId AddMaterial(std::string_view)          = 0;
	virtual MaterialId AddNewMaterial(MaterialType, MaterialId = MaterialId()) = 0;
	virtual void Clear()                                                       = 0;

	virtual Material& GetMaterial(MaterialId)                 = 0;
	virtual DiffuseMaterial& GetDiffuseMaterial(MaterialId)   = 0;
	virtual ParticleMaterial& GetParticleMaterial(MaterialId) = 0;
	virtual bool IsMaterialLoaded(MaterialId)                 = 0;
};

class NullMaterialManager : public IMaterialManager
{
public:
	MaterialId AddMaterial(std::string_view) override { return {}; }
	MaterialId AddNewMaterial(MaterialType, MaterialId = MaterialId()) override { return {}; }

	void Clear() override {}

	Material& GetMaterial(MaterialId) override { neko_assert(false, "Material Manager is null!"); }

	DiffuseMaterial& GetDiffuseMaterial(MaterialId) override
	{
		neko_assert(false, "Material Manager is null!");
	}

    ParticleMaterial& GetParticleMaterial(MaterialId) override
	{
		neko_assert(false, "Material Manager is null!");
	}

	bool IsMaterialLoaded(MaterialId) override { return false; }
};

class MaterialManager final : public IMaterialManager
{
public:
	MaterialManager();

	MaterialId AddMaterial(std::string_view materialPath) override;
	MaterialId AddNewMaterial(MaterialType materialType, MaterialId id = MaterialId()) override;
	void Clear() override;

	Material& GetMaterial(MaterialId resourceId) override;
    DiffuseMaterial& GetDiffuseMaterial(MaterialId resourceId) override;
    ParticleMaterial& GetParticleMaterial(MaterialId resourceId) override;
    bool IsMaterialLoaded(MaterialId resourceId) override;

private:
	std::unordered_map<MaterialId, DiffuseMaterial> diffuseMaterials_;

	//std::vector<ResourceId> skyboxMaterialIds_;
	//std::vector<SkyboxMaterial> skyboxMaterials_;

	//std::vector<ResourceId> trailMaterialIds_;
	//std::vector<TrailMaterial> trailMaterials_;

    std::unordered_map<MaterialId, ParticleMaterial> particleMaterials_;

	//ImageCube defaultImageCube_;
};

using MaterialManagerLocator = Locator<IMaterialManager, NullMaterialManager>;
}    // namespace neko::vk
