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
#include "graphics/texture.h"
#include "graphics/graphics.h"
#define STB_IMAGE_IMPLEMENTATION
#include <fmt/format.h>
#include <utils/json_utility.h>
#include "engine/engine.h"
#include "stb_image.h"
#include "utils/file_utility.h"

#ifdef EASY_PROFILE_USE
#include "easy/profiler.h"
#endif

namespace neko
{
Image::Image(Image&& image) noexcept
{
	data       = image.data;
	image.data = nullptr;
	width      = image.width;
	height     = image.height;
	nbChannels = image.nbChannels;
}

Image& Image::operator=(Image&& image) noexcept
{
	data       = image.data;
	image.data = nullptr;
	width      = image.width;
	height     = image.height;
	nbChannels = image.nbChannels;
	return *this;
}

Image::~Image() { Destroy(); }

void Image::Destroy()
{
	if (data) stbi_image_free(data);

	data   = nullptr;
	height = -1;
	width  = -1;
}

Image StbImageConvert(const BufferFile& imageFile, bool flipY, bool hdr)
{
#ifdef EASY_PROFILE_USE
	EASY_BLOCK("Convert Image");
#endif
	Image image;

	stbi_set_flip_vertically_on_load(flipY);
	if (hdr)
	{
		image.data =
			(unsigned char*) stbi_loadf_from_memory((unsigned char*) (imageFile.dataBuffer),
				imageFile.dataLength,
				&image.width,
				&image.height,
				&image.nbChannels,
				0);
	}
	else
	{
		image.data = stbi_load_from_memory((unsigned char*) (imageFile.dataBuffer),
			imageFile.dataLength,
			&image.width,
			&image.height,
			&image.nbChannels,
			0);
	}
	return image;
}
}    // namespace neko
