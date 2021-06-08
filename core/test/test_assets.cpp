/*
MIT License

Copyright (c) 2019 SAE Institute Switzerland AG

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
#include <engine/filesystem.h>
#include <gtest/gtest.h>
#include <fstream>
#include <iostream>
#include <mathematics/hash.h>
#include <sole.hpp>
#include "engine/engine.h"
#include "utils/file_utility.h"

TEST(Engine, TestUUIDToStringToUUID)
{
	const sole::uuid id = sole::uuid4();
	const auto idStr = id.str();
	const auto convertId = sole::rebuild(idStr);
	EXPECT_TRUE(id == convertId);
}

TEST(Engine, TestAssetImport)
{
	neko::Configuration config;
	neko::Filesystem filesystem;
	config.dataRootPath = "../";
	std::vector<std::string> filenames =
	{
		config.dataRootPath+"data/sprites/platform.jpg",
		config.dataRootPath+"data/sprites/wall.jpg",
		config.dataRootPath+"data/sprites/icons/icons8-road-48.png",
		config.dataRootPath+"fake/path/file.png",
		config.dataRootPath+"other/fake/path/file.png",
	};

	std::vector<neko::StringHash> fileHashes;
	fileHashes.reserve(filenames.size());
	for (auto& filename : filenames)
	{
		neko::BufferFile bufferFile = filesystem.LoadFile(filename);
		if (bufferFile.dataBuffer)
			fileHashes.push_back(
				neko::HashString(reinterpret_cast<const char*>(bufferFile.dataBuffer)));

		bufferFile.Destroy();
	}

	EXPECT_NE(fileHashes[0], fileHashes[1]);
	EXPECT_NE(fileHashes[1], fileHashes[2]);
	EXPECT_NE(fileHashes[2], fileHashes[0]);
}