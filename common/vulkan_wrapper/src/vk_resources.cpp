#include "vk/vk_resources.h"

#include "vk/material/material_manager.h"

namespace neko::vk
{
VkResources::VkResources(sdl::VulkanWindow* window) : vkWindow(window) { Inst = this; }

VkResources::~VkResources() {}

void VkResources::DestroyResources()
{
	vkDeviceWaitIdle(device);

	const auto& graphicsQueue = device.GetGraphicsQueue();
	vkQueueWaitIdle(graphicsQueue);
	vkDestroyPipelineCache(device, pipelineCache, nullptr);

	for (size_t i = 0; i < inFlightFences_.size(); i++)
	{
		vkDestroyFence(device, inFlightFences_[i], nullptr);
		vkDestroySemaphore(device, availableSemaphores_[i], nullptr);
		vkDestroySemaphore(device, finishedSemaphores_[i], nullptr);
	}
	
    vk::MaterialManagerLocator::get().Clear();

	imgui_.Destroy();
	renderer_->Destroy();

	for (auto& commandBuffer : commandBuffers_)
		commandBuffer.Destroy();

	for (const auto& commandPool : commandPools_)
		commandPool.second.Destroy();

	swapchain.Destroy();

	device.Destroy();
	surface.Destroy();
	instance.Destroy();
	commandBuffers_.clear();
	commandPools_.clear();
	inFlightFences_.clear();
	availableSemaphores_.clear();
	finishedSemaphores_.clear();
	attachments_.clear();
	//renderer_.release();
}

MaterialPipeline& VkResources::AddMaterialPipeline(
	const PipelineStage& pipelineStage, const GraphicsPipelineCreateInfo& pipelineCreate)
{
	return materialPipelineContainer_.AddMaterial(pipelineStage, pipelineCreate);
}

RenderStage& VkResources::GetRenderStage() const { return renderer_->GetRenderStage(); }

const RenderPass& VkResources::GetRenderPass() const { return renderer_->GetRenderPass(); }

CommandBuffer& VkResources::GetCurrentCmdBuffer()
{
	return commandBuffers_[swapchain.GetCurrentImageIndex()];
}

const CommandPool& VkResources::GetCurrentCmdPool()
{
	const std::thread::id threadId = std::this_thread::get_id();

	auto it = commandPools_.find(threadId);
	if (it != commandPools_.end()) return it->second;

	commandPools_.emplace(threadId, CommandPool());

	it = commandPools_.find(threadId);
	return it->second;
}

const IDescriptor& VkResources::GetAttachment(const std::string_view& name) const
{
    const auto hash = HashString(name);
    const auto it = attachments_.find(hash);
	neko_assert(it != attachments_.end(), "Attachment " << name << " doesn't exist");

	return it->second;
}

#ifdef NEKO_RAYTRACING
void VkResources::GetRaytracingFuncsPtr() const
{
    // Get the function pointers required for ray tracing
    vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(device, "vkGetAccelerationStructureDeviceAddressKHR"));
    vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(device, "vkGetAccelerationStructureBuildSizesKHR"));
    vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR"));
    vkBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device, "vkBuildAccelerationStructuresKHR"));
    vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresKHR"));
    vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR"));

    vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(device, "vkGetBufferDeviceAddressKHR"));

    vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(device, "vkCmdTraceRaysKHR"));
    vkGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupHandlesKHR"));
    vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesKHR"));
}
#endif
}    // namespace neko::vk