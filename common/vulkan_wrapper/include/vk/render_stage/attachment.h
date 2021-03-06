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
#include "vk/vk_include.h"

namespace neko::vk
{
struct Attachment
{
public:
	enum class Type : std::uint8_t
	{
		IMAGE = 0,
		DEPTH,
		SWAPCHAIN,
		NONE
	};

	Attachment(const std::uint32_t binding = 0,
		std::string_view name              = "",
		const Type type                    = Type::NONE,
		const bool multisampling           = false,
		const VkFormat format              = VK_FORMAT_B8G8R8A8_UNORM)
	   : binding(binding), name(name), type(type), multisampling(multisampling), format(format)
	{}

	std::uint32_t binding = 0;
	std::string name;
	Type type          = Type::NONE;
	bool multisampling = false;
	VkFormat format    = VK_FORMAT_B8G8R8A8_UNORM;
};
}    // namespace neko::vk