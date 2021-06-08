#pragma once
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
#include <functional>

#include <string>
#include <string_view>
#include <fstream>

#include "engine/jobsystem.h"

namespace neko
{
constexpr char GetOsSeparator()
{
#ifdef WIN32
    return '\\';
#else
    return '/';
#endif
}

bool FileExists(std::string_view filename);
bool IsRegularFile(std::string_view filename);
bool IsDirectory(std::string_view filename);

size_t CalculateFileSize(const std::string& filename);

std::string GetCurrentPath();

bool CreateDirectory(std::string_view dirname);
bool RemoveDirectory(std::string_view dirname, bool removeAll = true);
void IterateDirectory(std::string_view dirname,
	const std::function<void(const std::string_view)>& func,
	bool recursive = false);

std::string LoadFile(std::string_view path);
std::string LoadBinaries(std::string_view path);

std::string GetRelativePath(std::string_view path, std::string_view relative);
std::string GetFileParentPath(std::string_view path);
std::string GetStem(std::string_view path);

std::string GetFilename(std::string_view path);
std::string GetFilenameExtension(std::string_view path);

std::string LinkFolderAndFile(std::string_view folderPath, std::string_view filePath);
std::string MakeGeneric(std::string_view path);
void WriteStringToFile(const std::string& path, std::string_view content);
}    // namespace neko
