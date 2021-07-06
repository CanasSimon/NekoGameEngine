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
#ifdef NEKO_RAYTRACING
constexpr std::array<const char*, 8> kDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
	VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
	VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
	VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
	VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
	VK_KHR_SPIRV_1_4_EXTENSION_NAME,
	VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME};
#else
constexpr std::array<const char*, 1> kDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
#endif

class LogicalDevice final
{
public:
	LogicalDevice() = default;
	operator const VkDevice&() const { return device_; }

	void Init();
	void Destroy() const;

	[[nodiscard]] VkQueue GetGraphicsQueue() const { return graphicsQueue_; }
	[[nodiscard]] VkQueue GetPresentQueue() const { return presentQueue_; }

private:
	VkDevice device_ {};
	VkQueue graphicsQueue_ {};
	VkQueue presentQueue_ {};
	VkQueue computeQueue_ {};
};
}    // namespace neko::vk