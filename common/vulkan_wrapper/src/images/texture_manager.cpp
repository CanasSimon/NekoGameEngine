#include "vk/images/texture_manager.h"

namespace neko::vk
{
TextureManager::TextureManager() { TextureManagerLocator::provide(this); }

void TextureManager::Init() {}

void TextureManager::Update(seconds)
{
	while (!loaders_.empty())
	{
		auto& textureLoader = loaders_.front();
		if (textureLoader.HasErrors())
		{
			switch (textureLoader.error_)
			{
				case TextureLoader::TextureLoaderError::ASSET_LOADING_ERROR:
					logDebug(fmt::format(
						"[Error] Could not load texture {} from disk", textureLoader.path_));
					break;
				case TextureLoader::TextureLoaderError::DECOMPRESS_ERROR:
					logDebug(fmt::format(
						"[Error] Could not decompress texture {} from disk", textureLoader.path_));
					break;
				case TextureLoader::TextureLoaderError::UPLOAD_TO_GPU_ERROR:
					logDebug(fmt::format(
						"[Error] Could not upload texture {} from disk", textureLoader.path_));
					break;
				default: break;
			}
			loaders_.pop();
		}
		else if (textureLoader.IsDone())
		{
			textures_[textureLoader.textureId_] = textureLoader.texture_;
			loaders_.pop();
		}
		else
		{
			break;
		}
	}
}

void TextureManager::Destroy()
{
	for (auto& textureName : textures_) textureName.second.Destroy();
}

ResourceHash TextureManager::AddTexture(std::string_view path)
{
	return AddTexture(path, Texture::DEFAULT);
}

ResourceHash TextureManager::AddTexture(std::string_view path, Texture::Flags flags)
{
	const ResourceHash textureId = HashString(path);
	const auto it                = textures_.find(textureId);
	if (it != textures_.end()) return it->first;

	//const std::string metaPath = fmt::format("{}.meta", path.substr(0, path.size() - 4));
	//const json metaJson = LoadJson(metaPath);
	if (textureId == kInvalidTextureId)
	{
		logDebug("[Error] Invalid texture id on texture load");
		return textureId;
	}

	loaders_.push({path, textureId, flags});
	loaders_.back().Start();
	return textureId;
}

const Image2d* TextureManager::GetTexture(std::string_view texturePath) const
{
	return GetTexture(HashString(texturePath));
}

const Image2d* TextureManager::GetTexture(ResourceHash resourceId) const
{
	const auto it = textures_.find(resourceId);
	if (it != textures_.end()) { return &it->second; }
;
	return nullptr;
}

void TextureManager::Clear() { textures_.clear(); }
}    // namespace neko::vk