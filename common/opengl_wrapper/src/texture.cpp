/*
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
 */
#include "gl/texture.h"

#include <fmt/format.h>

#include "engine/engine.h"
#include "engine/log.h"
#include "graphics/texture.h"
#include "utils/file_utility.h"

#include "gl/gl_include.h"

#ifdef NEKO_PROFILE
#include "easy/profiler.h"
#endif

namespace neko::gl
{
void TextureManager::Destroy()
{
	for (auto& textureName : textures_)
	{
		DestroyTexture(textureName.second.name);
		textureName.second.name = INVALID_TEXTURE_NAME;
	}
}

TextureName StbCreateTexture(const std::string_view filename,
	const FilesystemInterface& filesystem,
	Texture::TextureFlags flags)
{
#ifdef NEKO_PROFILE
	EASY_BLOCK("Create Texture");
	EASY_BLOCK("Load From File");
#endif
	const std::string extension = GetFilenameExtension(filename);
	if (!filesystem.FileExists(filename))
	{
        LogError(fmt::format("Texture: {} does not exist", filename));
		return 0;
	}

	BufferFile textureFile = filesystem.LoadFile(filename);
#ifdef NEKO_PROFILE
	EASY_END_BLOCK;
#endif
	Image image = StbImageConvert(textureFile);
	textureFile.Destroy();
	if (image.data == nullptr)
	{
        LogError(fmt::format("Texture: cannot load {}", filename));
		return INVALID_TEXTURE_NAME;
	}

#ifdef NEKO_PROFILE
	EASY_BLOCK("Push Texture To GPU");
#endif
	TextureName texture;
	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D,
		GL_TEXTURE_WRAP_S,
		flags & Texture::CLAMP_WRAP ? GL_CLAMP_TO_EDGE : GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,
		GL_TEXTURE_WRAP_T,
		flags & Texture::CLAMP_WRAP ? GL_CLAMP_TO_EDGE : GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,
		GL_TEXTURE_MAG_FILTER,
		flags & Texture::SMOOTH_TEXTURE ? GL_LINEAR : GL_NEAREST);
	if (flags & Texture::MIPMAPS_TEXTURE)
	{
		glTexParameteri(GL_TEXTURE_2D,
			GL_TEXTURE_MIN_FILTER,
			flags & Texture::SMOOTH_TEXTURE ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D,
			GL_TEXTURE_MAG_FILTER,
			flags & Texture::SMOOTH_TEXTURE ? GL_LINEAR : GL_NEAREST);
	}
	if (extension == ".jpg" || extension == ".tga")
	{
		glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RGB,
			image.width,
			image.height,
			0,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			image.data);
	}
	else if (extension == ".png")
	{
		glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RGBA,
			image.width,
			image.height,
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			image.data);
	}
	else if (extension == ".hdr")
	{
		glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RGB16F,
			image.width,
			image.height,
			0,
			GL_RGB,
			GL_FLOAT,
			image.data);
	}
	if (flags & Texture::MIPMAPS_TEXTURE) { glGenerateMipmap(GL_TEXTURE_2D); }
	image.Destroy();
	return texture;
}

TextureName CreateTextureFromKTX(
	const std::string_view filename, const FilesystemInterface& filesystem)
{
#ifdef NEKO_PROFILE
	EASY_BLOCK("Load KTX Texture");
	EASY_BLOCK("Open File");
#endif
	BufferFile textureFile = filesystem.LoadFile(filename);
#ifdef NEKO_PROFILE
	EASY_END_BLOCK;
#endif

#ifdef NEKO_PROFILE
	EASY_BLOCK("Create KTX from memory");
#endif
	gli::gl glProfile(gli::gl::PROFILE_ES30);

	auto texture =
		gli::load(reinterpret_cast<const char*>(textureFile.dataBuffer), textureFile.dataLength);
	if (texture.empty())
	{
		LogDebug("Could not load texture with GLI");
		return 0;
	}
	const gli::gl::format format = glProfile.translate(texture.format(), texture.swizzles());

	GLenum target = glProfile.translate(texture.target());
	LogDebug(fmt::format("texture format: {}, texture target {}, is compressed {}",
		(int) texture.format(),
		(int) texture.target(),
		is_compressed(texture.format())));
#ifdef NEKO_PROFILE
	EASY_END_BLOCK;
#endif

#ifdef NEKO_PROFILE
	EASY_BLOCK("Upload Texture to GPU");
#endif
	TextureName textureName = 0;
	glGenTextures(1, &textureName);
	glBindTexture(target, textureName);

	glCheckError();
	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(texture.levels() - 1));

	glCheckError();
	Vec3GLi extent {texture.extent()};
	glTexStorage2D(
		target, static_cast<GLint>(texture.levels()), format.Internal, extent.x, extent.y);

	glCheckError();
	for (std::size_t level = 0; level < texture.levels(); ++level)
	{
        Vec3GLi levelExtent(texture.extent(level));
		if (gli::is_compressed(texture.format()))
		{
			glCompressedTexSubImage2D(target,
				static_cast<GLint>(level),
				0,
				0,
				levelExtent.x,
				levelExtent.y,
				format.Internal,
				static_cast<GLsizei>(texture.size(level)),
				texture.data(0, 0, level));
		}
		else
		{
			glTexSubImage2D(target,
				static_cast<GLint>(level),
				0,
				0,
				levelExtent.x,
				levelExtent.y,
				format.Internal,
				static_cast<GLsizei>(texture.size(level)),
				texture.data(0, 0, level));
		}
		glCheckError();
	}
	glCheckError();
#ifdef NEKO_PROFILE
	EASY_END_BLOCK;
#endif
	return textureName;
}

TextureName LoadCubemap(
	std::vector<std::string> facesFilename, const FilesystemInterface& filesystem)
{
    TextureName textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    for (unsigned i = 0; i < facesFilename.size(); i++)
    {
		BufferFile textureFile = filesystem.LoadFile(facesFilename[i]);
		Image image            = StbImageConvert(textureFile);
		textureFile.Destroy();
		if (image.data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				image.nbChannels == 3 ? GL_RGB : GL_RGBA,
				image.width,
				image.height,
				0,
				image.nbChannels == 3 ? GL_RGB : GL_RGBA,
				GL_UNSIGNED_BYTE,
				image.data);
		}
		else
		{
			LogError(fmt::format("Cubemap tex failed to load at path: {}", facesFilename[i]));
		}

		image.Destroy();
	}

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glCheckError();
	return textureID;
}

void DestroyTexture(TextureName textureName)
{
	glDeleteTextures(1, &textureName);
	textureName = INVALID_TEXTURE_NAME;
}

TextureId TextureManager::LoadTexture(std::string_view path, Texture::TextureFlags flags)
{
	const auto it = pathMap_.find(path.data());
	if (it != pathMap_.end()) return it->second;

	const std::string metaPath = fmt::format("{}.meta", path);
	const json metaJson        = LoadJson(metaPath);
	TextureId textureId        = INVALID_TEXTURE_ID;
	std::string ktxPath;
	if (CheckJsonExists(metaJson, "uuid"))
	{
		textureId = sole::rebuild(metaJson["uuid"].get<std::string>());
	}
	else
	{
        LogError(fmt::format("Could not find texture id in json file {}", metaPath));
		return textureId;
	}

	if (CheckJsonExists(metaJson, "ktx_path")) { ktxPath = metaJson["ktx_path"]; }
	else
	{
        LogError("Could not find ktx path in json file");
		return INVALID_TEXTURE_ID;
	}

	if (textureId == INVALID_TEXTURE_ID)
	{
        LogError("Invalid texture id on texture load");
		return textureId;
	}

	const auto& config = BasicEngine::GetInstance()->GetConfig();
	loaders_.push(TextureLoader {config.dataRootPath + ktxPath, textureId, flags});
	loaders_.back().Start();
	pathMap_[path.data()] = textureId;
	return textureId;
}

const Texture* TextureManager::GetTexture(TextureId index) const
{
	const auto it = textures_.find(index);
	if (it != textures_.end()) return &it->second;
	return nullptr;
}

bool TextureManager::IsTextureLoaded(TextureId textureId) const
{
	const auto it = textures_.find(textureId);
	return it != textures_.end();
}

void TextureManager::Init() { TextureManagerLocator::provide(this); }

void TextureManager::Update(seconds)
{
	while (!loaders_.empty())
	{
		auto& textureLoader = loaders_.front();
		if (textureLoader.HasErrors())
		{
			switch (textureLoader.GetErrors())
			{
				case TextureLoader::TextureLoaderError::ASSET_LOADING_ERROR:
					LogError(fmt::format(
						"Could not load texture {} from disk", textureLoader.GetPath()));
					break;
				case TextureLoader::TextureLoaderError::DECOMPRESS_ERROR:
					LogError(fmt::format(
						"Could not decompress texture {} from disk", textureLoader.GetPath()));
					break;
				case TextureLoader::TextureLoaderError::UPLOAD_TO_GPU_ERROR:
					LogError(fmt::format(
						"Could not upload texture {} from disk", textureLoader.GetPath()));
					break;
				default: break;
			}
			loaders_.pop();
		}
		else if (textureLoader.IsDone())
		{
			textures_[textureLoader.GetTextureId()] = textureLoader.GetTexture();
			loaders_.pop();
		}
		else
		{
			break;
		}
	}
}

TextureManager::TextureManager() : filesystem_(BasicEngine::GetInstance()->GetFilesystem()) {}

TextureName TextureManager::GetTextureName(TextureId textureId) const
{
	const auto* texture = GetTexture(textureId);
	if (texture == nullptr) return INVALID_TEXTURE_NAME;
	return texture->name;
}

TextureLoader::TextureLoader(
	std::string_view path, TextureId textureId, Texture::TextureFlags flags)
   : filesystem_(BasicEngine::GetInstance()->GetFilesystem()),
	 textureId_(textureId),
	 path_(path),
	 flags_(flags),
	 loadingTextureJob_([this]() { LoadTexture(); }),
	 decompressTextureJob_([this]() { DecompressTexture(); }),
	 uploadToGLJob_([this]() { UploadToGL(); })
{}

TextureLoader::TextureLoader(TextureLoader&& textureLoader) noexcept
   : filesystem_(BasicEngine::GetInstance()->GetFilesystem()),
	 textureId_(textureLoader.textureId_),
	 path_(std::move(textureLoader.path_)),
	 flags_(textureLoader.flags_),
	 loadingTextureJob_([this]() { LoadTexture(); }),
	 decompressTextureJob_([this]() { DecompressTexture(); }),
	 uploadToGLJob_([this]() { UploadToGL(); })
{}

void TextureLoader::Start()
{
    BasicEngine::GetInstance()->ScheduleJob(&loadingTextureJob_, JobThreadType::RESOURCE_THREAD);
}

void TextureLoader::LoadTexture()
{
#ifdef NEKO_PROFILE
	EASY_BLOCK("Load KTX from disk");
#endif
	bufferFile_ = filesystem_.get().LoadFile(path_);
	if (bufferFile_.dataBuffer == nullptr)
	{
		error_ = TextureLoaderError::ASSET_LOADING_ERROR;
		return;
	}

	BasicEngine::GetInstance()->ScheduleJob(&decompressTextureJob_, JobThreadType::OTHER_THREAD);
}

void TextureLoader::DecompressTexture()
{
    {
#ifdef NEKO_PROFILE
		EASY_BLOCK("Create KTX from memory");
#endif
		texture_.gliTexture = gli::load(
			reinterpret_cast<const char*>(bufferFile_.dataBuffer), bufferFile_.dataLength);
	}

	if (texture_.gliTexture.empty())
	{
        LogError("OpenGLI error while opening KTX content");
		error_ = TextureLoaderError::DECOMPRESS_ERROR;
		return;
	}

	RendererLocator::get().AddPreRenderJob(&uploadToGLJob_);
}

void TextureLoader::UploadToGL()
{
#ifdef NEKO_PROFILE
	EASY_BLOCK("Upload KTX Texture to GPU");
#endif
	gli::gl glProfile(gli::gl::PROFILE_GL33);

	auto& texture           = texture_.gliTexture;
	const auto isCompressed = gli::is_compressed(texture.format());
	const auto format       = glProfile.translate(texture.format(), texture.swizzles());
	const auto target       = glProfile.translate(texture.target());
	glGenTextures(1, &texture_.name);
	glBindTexture(target, texture_.name);

	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(texture.levels() - 1));

    Vec3GLi extent {texture.extent()};
	glTexStorage2D(
		target, static_cast<GLint>(texture.levels()), format.Internal, extent.x, extent.y);
	for (std::size_t level = 0; level < texture.levels(); ++level)
	{
        Vec3GLi levelExtent(texture.extent(level));
		if (isCompressed)
		{
			glCompressedTexSubImage2D(target,
				static_cast<GLint>(level),
				0,
				0,
				levelExtent.x,
				levelExtent.y,
				format.Internal,
				static_cast<GLsizei>(texture.size(level)),
				texture.data(0, 0, level));
		}
		else
		{
			glTexSubImage2D(target,
				static_cast<GLint>(level),
				0,
				0,
				levelExtent.x,
				levelExtent.y,
				format.Internal,
				static_cast<GLsizei>(texture.size(level)),
				texture.data(0, 0, level));
		}
	}

	glCheckError();
}
}    // namespace neko::gl
