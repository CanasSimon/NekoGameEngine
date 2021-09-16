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
#include "vk/commands/command_pool.h"
#include "vk/commands/light_command_buffer.h"
#include "vk/commands/model_command_buffer.h"
#include "vk/commands/particle_command_buffer.h"
#include "vk/core/instance.h"
#include "vk/core/logical_device.h"
#include "vk/core/physical_device.h"
#include "vk/pipelines/material_pipeline_container.h"
#include "vk/renderers/renderer.h"
#include "vk/vk_imgui.h"
#include "vk/vulkan_window.h"

namespace neko::vk
{
class VkResources
{
public:
	explicit VkResources(sdl::VulkanWindow* window);
	~VkResources();
	void DestroyResources();

	/// Add a new material pipeline to the render queue
	[[nodiscard]] MaterialPipeline& AddMaterialPipeline(
		const PipelineStage& pipelineStage, const GraphicsPipelineCreateInfo& pipelineCreate);

	[[nodiscard]] RenderStage& GetRenderStage() const;
	[[nodiscard]] const RenderPass& GetRenderPass() const;
	[[nodiscard]] CommandBuffer& GetCurrentCmdBuffer();
	[[nodiscard]] const CommandPool& GetCurrentCmdPool();
    [[nodiscard]] const IDescriptor& GetAttachment(const std::string_view& name) const;

	[[nodiscard]] std::uint8_t GetViewportCount() const { return viewportCount_; }
	void SetViewportCount(std::uint8_t count) { viewportCount_ = count; }

#ifdef NEKO_RAYTRACING
	[[nodiscard]] VkPhysicalDeviceRayTracingPipelinePropertiesKHR GetRayPipelineProperties() const
	{
		return rayTracingPipelineProperties;
	}

	[[nodiscard]] VkPhysicalDeviceAccelerationStructureFeaturesKHR GetAccelerationStructProperties() const
	{
		return accelerationStructureFeatures;
	}

	void GetRaytracingFuncsPtr() const;
#endif

	static VkResources* Inst;

	std::unique_ptr<sdl::VulkanWindow> vkWindow = nullptr;

	Instance instance;
	Surface surface;
	PhysicalDevice gpu;
	LogicalDevice device;

	Swapchain swapchain {};

	std::array<ModelCommandBuffer, 4> modelCommandBuffers;
	LightCommandBuffer lightCommandBuffer;
	ParticleCommandBuffer particleCommandBuffer;

	VkPipelineCache pipelineCache {};

#ifdef NEKO_RAYTRACING
    // Function pointers for ray tracing related stuff
    static PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
    static PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
    static PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
    static PFN_vkBuildAccelerationStructuresKHR vkBuildAccelerationStructuresKHR;
    static PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
    static PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR;

    static PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR;

    static PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;
    static PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR;
    static PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR;
#endif

protected:
	bool isFramebufferResized_ = false;

	std::uint8_t viewportCount_ = 1;
	std::vector<CommandBuffer> commandBuffers_ {};
	std::map<std::thread::id, CommandPool> commandPools_ {};

	std::uint32_t currentFrame_ = 0;
	std::vector<VkFence> inFlightFences_ {};
	std::vector<VkSemaphore> availableSemaphores_ {};
	std::vector<VkSemaphore> finishedSemaphores_ {};

	std::map<StringHash, const IDescriptor&> attachments_ {};

	MaterialPipelineContainer materialPipelineContainer_ {};

	VkImGui imgui_;
	std::unique_ptr<IRenderer> renderer_{};

#ifdef NEKO_RAYTRACING
	// Available features and properties
	VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties {};
	VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures {};
#endif
};

// Init static members
inline VkResources* VkResources::Inst = nullptr;

#ifdef NEKO_RAYTRACING
inline PFN_vkGetAccelerationStructureDeviceAddressKHR VkResources::vkGetAccelerationStructureDeviceAddressKHR = nullptr;
inline PFN_vkGetAccelerationStructureBuildSizesKHR VkResources::vkGetAccelerationStructureBuildSizesKHR = nullptr;
inline PFN_vkCreateAccelerationStructureKHR VkResources::vkCreateAccelerationStructureKHR = nullptr;
inline PFN_vkBuildAccelerationStructuresKHR VkResources::vkBuildAccelerationStructuresKHR = nullptr;
inline PFN_vkCmdBuildAccelerationStructuresKHR VkResources::vkCmdBuildAccelerationStructuresKHR = nullptr;
inline PFN_vkDestroyAccelerationStructureKHR VkResources::vkDestroyAccelerationStructureKHR = nullptr;
inline PFN_vkGetBufferDeviceAddressKHR VkResources::vkGetBufferDeviceAddressKHR = nullptr;
inline PFN_vkCmdTraceRaysKHR VkResources::vkCmdTraceRaysKHR = nullptr;
inline PFN_vkGetRayTracingShaderGroupHandlesKHR VkResources::vkGetRayTracingShaderGroupHandlesKHR = nullptr;
inline PFN_vkCreateRayTracingPipelinesKHR VkResources::vkCreateRayTracingPipelinesKHR = nullptr;
#endif
}    // namespace neko::vk
