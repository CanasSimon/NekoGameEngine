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
#include "mathematics/matrix.h"

#include "vk/buffers/buffer.h"
#include "vk/descriptors/descriptor_interface.h"

namespace neko::vk
{
struct UniformBufferObject
{
	Mat4f model = Mat4f::Identity;
	Mat4f view  = Mat4f::Identity;
	Mat4f proj  = Mat4f::Identity;
};

struct UniformBuffer final : IDescriptor, Buffer
{
	explicit UniformBuffer(VkDeviceSize size, const std::vector<char>& uniformData = {});

	void Update(const std::vector<char>& newUniformData) const;
	void Destroy() const override;

	static VkDescriptorSetLayoutBinding GetDescriptorSetLayout(
		std::uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stage);

	[[nodiscard]] WriteDescriptorSet GetWriteDescriptor(
		std::uint32_t binding, VkDescriptorType descriptorType) const override;
};
}    // namespace neko::vk