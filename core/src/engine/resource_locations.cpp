#include "engine/resource_locations.h"

#include "engine/engine.h"
#include "engine/log.h"
#include "utils/file_utility.h"

#ifdef __APPLE__
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#elif WIN32
#include <filesystem>
namespace fs = std::filesystem;
#elif defined(__linux__) || defined(EMSCRIPTEN)
#include <filesystem>
namespace fs = std::filesystem;
#endif

namespace neko
{
std::string GetBanksFolderPath()
{
	const std::string dataRoot = BasicEngine::GetInstance()->GetConfig().dataRootPath;
	return dataRoot + kBanksFolderName;
}

std::vector<std::string> GetBankPaths()
{
	const std::string banksFolder =
		BasicEngine::GetInstance()->GetConfig().dataRootPath + kBanksFolderName;
	if (IsDirectory(banksFolder))
	{
		std::vector<std::string> bankPaths;
		for (auto& p : fs::directory_iterator(banksFolder))
			bankPaths.emplace_back(p.path().generic_string());

		return bankPaths;
	}

	logError("[Error] The FMOD banks folder doesn't exist!");
	return {};
}

std::string GetFontsFolderPath()
{
	const std::string dataRoot = BasicEngine::GetInstance()->GetConfig().dataRootPath;
	return dataRoot + kFontsFolderName;
}

std::string GetModelsFolderPath()
{
	const std::string dataRoot = BasicEngine::GetInstance()->GetConfig().dataRootPath;
	return dataRoot + kModelsFolderName;
}

std::string GetScenesFolderPath()
{
	const std::string dataRoot = BasicEngine::GetInstance()->GetConfig().dataRootPath;
	return dataRoot + kScenesFolderName;
}

std::string GetShadersFolderPath()
{
	const std::string dataRoot = BasicEngine::GetInstance()->GetConfig().dataRootPath;
	return dataRoot + kShadersFolderName;
}

std::string GetGlShadersFolderPath()
{
	const std::string dataRoot = BasicEngine::GetInstance()->GetConfig().dataRootPath;
	return dataRoot + kGlShadersFolderName;
}

std::string GetVkShadersFolderPath()
{
	const std::string dataRoot = BasicEngine::GetInstance()->GetConfig().dataRootPath;
	return dataRoot + kVkShadersFolderName;
}

std::string GetSpritesFolderPath()
{
	const std::string dataRoot = BasicEngine::GetInstance()->GetConfig().dataRootPath;
	return dataRoot + kSpritesFolderName;
}
}    // namespace neko