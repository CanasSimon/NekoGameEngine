#pragma once

#include <string_view>
#include "engine/jobsystem.h"

#if defined(WIN32) || defined(__linux__)
#define STD_FILESYSTEM
#endif

namespace neko
{
/**
 * \brief RAII structure, please Destroy it
 */
struct BufferFile
{
	BufferFile() = default;
	~BufferFile();
	BufferFile(BufferFile&& bufferFile) noexcept;
	BufferFile& operator          =(BufferFile&& bufferFile) noexcept;
	BufferFile(const BufferFile&) = delete;
	BufferFile& operator=(const BufferFile&) = delete;

	unsigned char* dataBuffer = nullptr;
	size_t dataLength         = 0;
	void Destroy();
};

/// Simple interface to a filesystem, useful to define
/// specialized filesystem like physfs
class FilesystemInterface
{
public:
	virtual ~FilesystemInterface()                                         = default;
	[[nodiscard]] virtual BufferFile LoadFile(std::string_view path) const = 0;

	[[nodiscard]] virtual bool FileExists(std::string_view) const    = 0;
	[[nodiscard]] virtual bool IsRegularFile(std::string_view) const = 0;
	[[nodiscard]] virtual bool IsDirectory(std::string_view) const   = 0;
};

class LoadingAssetJob : public Job
{
public:
	explicit LoadingAssetJob(const FilesystemInterface&);

    void Reset() override;

    const BufferFile& GetBufferFile() const { return bufferFile_; }

	std::string GetFilePath() const { return filePath_; }
    void SetFilePath(std::string_view path);

private:
	const FilesystemInterface& filesystem_;
	std::string filePath_;
	BufferFile bufferFile_;
};

class Filesystem : public FilesystemInterface
{
public:
	[[nodiscard]] BufferFile LoadFile(std::string_view path) const override;

    [[nodiscard]] bool FileExists(std::string_view view) const override;
    [[nodiscard]] bool IsRegularFile(std::string_view view) const override;
    [[nodiscard]] bool IsDirectory(std::string_view view) const override;
};
}    // namespace neko