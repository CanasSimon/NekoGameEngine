#include "vk/graphics.h"

#include "engine/resource_locations.h"
#include "utils/file_utility.h"

#include "sdl_engine/sdl_camera.h"

#include "vk/subrenderers/subrenderer_opaque.h"
#include "vk/vk_utilities.h"

#ifdef NEKO_PROFILE
#include "easy/profiler.h"
#endif

namespace neko::vk
{
VkRenderer::VkRenderer(sdl::VulkanWindow* window) : Renderer(), VkResources(window)
{
	instance.Init();
	surface.Init(*window->GetWindow());
	gpu.Init();
	surface.SetFormat();
	device.Init();

	commandPools_.emplace(std::this_thread::get_id(), CommandPool());

	CreatePipelineCache();

	swapchain.Init(swapchain);

	// Get properties and features
    rayTracingPipelineProperties.sType  = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
    accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;

    VkPhysicalDeviceProperties2 deviceProperties2{};
    deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    deviceProperties2.pNext = &rayTracingPipelineProperties;
    vkGetPhysicalDeviceProperties2(gpu, &deviceProperties2);

    VkPhysicalDeviceFeatures2 deviceFeatures2{};
    deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    deviceFeatures2.pNext = &accelerationStructureFeatures;
    vkGetPhysicalDeviceFeatures2(gpu, &deviceFeatures2);

	GetRaytracingFuncsPtr();

    sceneModelId_ = ModelManagerLocator::get().LoadModel(GetModelsFolderPath() + "vk_statue/vk_statue.obj");

	const VkExtent2D size = swapchain.GetExtent();
	CreateStorageImage(kFormat, { size.width, size.height, 1 });
	CreateUniformBuffer();
	CreateRayTracingPipeline();
	CreateShaderBindingTables();
	CreateDescriptorSets();
	BuildCommandBuffers();
}

void VkRenderer::ClearScreen()
{
#ifdef NEKO_PROFILE
	EASY_BLOCK("Clear Screen");
#endif
}

void VkRenderer::BeforeRender()
{
	/*const std::uint32_t windowFlags = SDL_GetWindowFlags(vkWindow->GetWindow());
	if (renderer_ == nullptr || windowFlags & SDL_WINDOW_MINIMIZED) return;

	if (!renderer_->HasStarted())
	{
		ResetRenderStages();
		renderer_->Start();
		imgui_.Init();
	}

	const VkResult res = swapchain.AcquireNextImage(
		availableSemaphores_[currentFrame_], inFlightFences_[currentFrame_]);
	if (res == VK_ERROR_OUT_OF_DATE_KHR)
	{
		RecreateSwapChain();
		return;
	}

	if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) return;

	RenderStage& renderStage = renderer_->GetRenderStage();
	renderStage.Update();

	if (!StartRenderPass(renderStage)) return;*/

	Renderer::BeforeRender();
}

void VkRenderer::AfterRender()
{
	Renderer::AfterRender();

    DrawRaytraced();

	/*PipelineStage stage;
	RenderStage& renderStage = renderer_->GetRenderStage();
	CommandBuffer& commandBuffer = GetCurrentCmdBuffer();
	for (const auto& subpass : renderStage.GetSubpasses())
	{
		stage.subPassId = subpass.binding;

		// Renders subpass subrender pipelines.
		renderer_->GetRendererContainer().RenderStage(stage, commandBuffer);

		if (subpass.binding != renderStage.GetSubpasses().back().binding)
			vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
	}

	VkImGui::Render(commandBuffer);
	EndRenderPass(renderStage);

	lightCommandBuffer.Clear();
	stage.renderPassId++;*/
}

void VkRenderer::RenderAll()
{
    for (auto* renderCommand : currentCommandBuffer_) renderCommand->Render();
}

void VkRenderer::DrawRaytraced()
{
	CreateDescriptorSets();
    UpdateUniformBuffers();

    swapchain.AcquireNextImage(availableSemaphores_[currentFrame_], inFlightFences_[currentFrame_]);
    CommandBuffer& commandBuffer = GetCurrentCmdBuffer();
	commandBuffer.Submit(availableSemaphores_[currentFrame_],
		finishedSemaphores_[currentFrame_],
		inFlightFences_[currentFrame_]);

	VkQueue presentQueue = device.GetPresentQueue();
    swapchain.QueuePresent(presentQueue, finishedSemaphores_[currentFrame_]);
}

bool VkRenderer::StartRenderPass(RenderStage& renderStage)
{
	if (renderStage.IsOutOfDate())
	{
		RecreatePass(renderStage);
		return false;
	}

	CommandBuffer& currentCmdBuffer = GetCurrentCmdBuffer();
	if (!currentCmdBuffer.IsRunning())
	{
		vkWaitForFences(device,
			1,
			&inFlightFences_[currentFrame_],
			VK_TRUE,
			std::numeric_limits<uint64_t>::max());
		currentCmdBuffer.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
	}

	VkRect2D renderArea;
	renderArea.offset = {0, 0};
	renderArea.extent = {static_cast<uint32_t>(renderStage.GetSize().x),
		static_cast<uint32_t>(renderStage.GetSize().y)};

	VkRect2D scissor;
	scissor.offset = {0, 0};
	scissor.extent = renderArea.extent;
	vkCmdSetScissor(currentCmdBuffer, 0, 1, &scissor);

	auto clearValues                     = renderStage.GetClearValues();
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass            = renderStage.GetRenderPass();
	renderPassInfo.framebuffer =
		renderStage.GetActiveFramebuffer(swapchain.GetCurrentImageIndex());
	renderPassInfo.renderArea      = renderArea;
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues    = clearValues.data();
	vkCmdBeginRenderPass(currentCmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	return true;
}

void VkRenderer::EndRenderPass(const RenderStage& renderStage)
{
	VkQueue presentQueue            = device.GetPresentQueue();
	CommandBuffer& currentCmdBuffer = GetCurrentCmdBuffer();
	vkCmdEndRenderPass(currentCmdBuffer);

	if (!renderStage.HasSwapchain()) return;

	currentCmdBuffer.Submit(availableSemaphores_[currentFrame_],
		finishedSemaphores_[currentFrame_],
		inFlightFences_[currentFrame_]);

	const VkResult res = swapchain.QueuePresent(presentQueue, finishedSemaphores_[currentFrame_]);
	vkCheckError(res, "Failed to presents swapchain image");

	if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) isFramebufferResized_ = true;
	currentFrame_ = (currentFrame_ + 1) % swapchain.GetImageCount();
}

void VkRenderer::ResetRenderStages()
{
	RecreateSwapChain();

	if (inFlightFences_.size() != swapchain.GetImageCount()) RecreateCommandBuffers();

	renderer_->GetRenderStage().Rebuild(swapchain);

	RecreateAttachments();
}

void VkRenderer::RecreateSwapChain()
{
	vkWindow->MinimizedLoop();

	vkDeviceWaitIdle(device);

	swapchain.Init(swapchain);

	RecreateCommandBuffers();

	if (ImGui::GetCurrentContext()) VkImGui::OnWindowResize();
}

void VkRenderer::RecreateCommandBuffers()
{
	for (size_t i = 0; i < inFlightFences_.size(); i++)
	{
		vkDestroyFence(device, inFlightFences_[i], nullptr);
		vkDestroySemaphore(device, availableSemaphores_[i], nullptr);
		vkDestroySemaphore(device, finishedSemaphores_[i], nullptr);
	}

	const std::size_t imageCount = swapchain.GetImageCount();
	availableSemaphores_.resize(imageCount);
	finishedSemaphores_.resize(imageCount);
	inFlightFences_.resize(imageCount);
	commandBuffers_.clear();
	commandBuffers_.reserve(imageCount);

	VkSemaphoreCreateInfo semaphoreInfo {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	for (size_t i = 0; i < inFlightFences_.size(); i++)
	{
		VkResult res = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &availableSemaphores_[i]);
		vkCheckError(res, "Could not create semaphore!");

		res = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &finishedSemaphores_[i]);
		vkCheckError(res, "Could not create semaphore!");

		res = vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences_[i]);
		vkCheckError(res, "Could not create fence!");

		commandBuffers_.emplace_back(false);
	}
}

void VkRenderer::RecreatePass(RenderStage& renderStage)
{
	VkQueue graphicQueue           = device.GetGraphicsQueue();
	const Vec2u& size              = BasicEngine::GetInstance()->GetConfig().windowSize;
	const VkExtent2D displayExtent = {static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y)};

	vkQueueWaitIdle(graphicQueue);

	if (renderStage.HasSwapchain() &&
		(isFramebufferResized_ || !swapchain.CompareExtent(displayExtent)))
	{
		RecreateSwapChain();
	}

	renderStage.Rebuild(swapchain);
	RecreateAttachments();
}

void VkRenderer::RecreateAttachments()
{
	attachments_.clear();

	const auto& descriptors = renderer_->GetRenderStage().GetDescriptors();
	attachments_.insert(descriptors.begin(), descriptors.end());
}

void VkRenderer::CreatePipelineCache()
{
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &pipelineCache);
}

void VkRenderer::SetWindow(std::unique_ptr<sdl::VulkanWindow> window)
{
	vkWindow = std::move(window);
}

void VkRenderer::SetRenderer(std::unique_ptr<IRenderer>&& newRenderer)
{
	renderer_ = std::move(newRenderer);
	renderer_->Init();
}

void VkRenderer::Destroy()
{
	Renderer::Destroy();
	DestroyResources();
}

VkPipelineShaderStageCreateInfo VkRenderer::LoadShader(std::string_view filename, VkShaderStageFlagBits stage)
{
	std::string data = LoadFile(filename);

	VkShaderModule shaderModule;
	VkShaderModuleCreateInfo moduleCreateInfo {};
	moduleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.codeSize = data.size();
	moduleCreateInfo.pCode    = (std::uint32_t*) data.data();
	vkCreateShaderModule(device, &moduleCreateInfo, nullptr, &shaderModule);

	VkPipelineShaderStageCreateInfo shaderStage = {};
	shaderStage.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.stage  = stage;
	shaderStage.module = shaderModule;
	shaderStage.pName  = "main";

	assert(shaderStage.module);
	shaderModules_.push_back(shaderStage.module);
	return shaderStage;
}

void VkRenderer::CreateStorageImage(VkFormat format, VkExtent3D extent)
{
	// Release resources if image is to be recreated
	if (storageImage_.image)
	{
		vkDestroyImageView(device, storageImage_.view, nullptr);
		vkDestroyImage(device, storageImage_.image, nullptr);
		vkFreeMemory(device, storageImage_.memory, nullptr);
		storageImage_ = {};
	}

	VkImageCreateInfo image {};
	image.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image.imageType     = VK_IMAGE_TYPE_2D;
	image.format        = format;
	image.extent        = extent;
	image.mipLevels     = 1;
	image.arrayLayers   = 1;
	image.samples       = VK_SAMPLE_COUNT_1_BIT;
	image.tiling        = VK_IMAGE_TILING_OPTIMAL;
	image.usage         = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	vkCreateImage(device, &image, nullptr, &storageImage_.image);

	VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(device, storageImage_.image, &memReqs);

	VkMemoryAllocateInfo memoryAllocateInfo {};
	memoryAllocateInfo.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memReqs.size;
	memoryAllocateInfo.memoryTypeIndex = gpu.GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &storageImage_.memory);
    vkBindImageMemory(device, storageImage_.image, storageImage_.memory, 0);

	VkImageViewCreateInfo colorImageView {};
	colorImageView.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	colorImageView.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
	colorImageView.format                          = format;
	colorImageView.subresourceRange                = {};
	colorImageView.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	colorImageView.subresourceRange.baseMipLevel   = 0;
	colorImageView.subresourceRange.levelCount     = 1;
	colorImageView.subresourceRange.baseArrayLayer = 0;
	colorImageView.subresourceRange.layerCount     = 1;
	colorImageView.image                           = storageImage_.image;
	vkCreateImageView(device, &colorImageView, nullptr, &storageImage_.view);

	CommandBuffer commandBuffer(true);
	commandBuffer.SetImageLayout(storageImage_.image,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_GENERAL,
		{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
	commandBuffer.SubmitIdle();
}

void VkRenderer::CreateUniformBuffer()
{
	ubo_ = Buffer(sizeof(uniformData_),
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&uniformData_);
    ubo_.MapMemory();

    UpdateUniformBuffers();
}

void VkRenderer::UpdateUniformBuffers()
{
	const auto& cameras = sdl::MultiCameraLocator::get();

	const Mat4f view = cameras.GenerateViewMatrix(0);
	Mat4f proj       = cameras.GenerateProjectionMatrix(0);
	proj[1][1] *= -1.0f;

	uniformData_.projInverse = proj.Inverse();
	uniformData_.viewInverse = view.Inverse();
	uniformData_.lightPos    = Vec4f(0.5f, 0.5f, 0.5f, 0.0f);

	// Pass the vertex size to the shader for unpacking vertices
	uniformData_.vertexSize = sizeof(Vertex);
	std::memcpy(ubo_.mapped, &uniformData_, sizeof(uniformData_));
}

void VkRenderer::CreateRayTracingPipeline()
{
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
        // Binding 0: Acceleration structure
        DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 0),
        // Binding 1: Storage image
        DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_KHR, 1),
        // Binding 2: Uniform buffer
        DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR, 2),
        // Binding 3: Vertex buffer
        DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 3),
        // Binding 4: Index buffer
        DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 4),
    };

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI {};
	descriptorSetLayoutCI.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCI.pBindings    = setLayoutBindings.data();
	descriptorSetLayoutCI.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
	vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCI, nullptr, &descriptorSetLayout_);

	VkPipelineLayoutCreateInfo pPipelineLayoutCI {};
	pPipelineLayoutCI.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pPipelineLayoutCI.setLayoutCount = 1;
	pPipelineLayoutCI.pSetLayouts    = &descriptorSetLayout_;
	vkCreatePipelineLayout(device, &pPipelineLayoutCI, nullptr, &pipelineLayout_);

	/*
        Setup ray tracing shader groups
    */
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

    // Ray generation group
    {
        shaderStages.push_back(LoadShader(GetShadersFolderPath() + "raytracing/raygen.rgen.spv", VK_SHADER_STAGE_RAYGEN_BIT_KHR));

		VkRayTracingShaderGroupCreateInfoKHR shaderGroup {};
		shaderGroup.sType              = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		shaderGroup.type               = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		shaderGroup.generalShader      = static_cast<uint32_t>(shaderStages.size()) - 1;
		shaderGroup.closestHitShader   = VK_SHADER_UNUSED_KHR;
		shaderGroup.anyHitShader       = VK_SHADER_UNUSED_KHR;
		shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
		shaderGroups.push_back(shaderGroup);
	}

    // Miss group
    {
        shaderStages.push_back(LoadShader(GetShadersFolderPath() + "raytracing/miss.rmiss.spv", VK_SHADER_STAGE_MISS_BIT_KHR));
		VkRayTracingShaderGroupCreateInfoKHR shaderGroup {};
		shaderGroup.sType              = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		shaderGroup.type               = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		shaderGroup.generalShader      = static_cast<uint32_t>(shaderStages.size()) - 1;
		shaderGroup.closestHitShader   = VK_SHADER_UNUSED_KHR;
		shaderGroup.anyHitShader       = VK_SHADER_UNUSED_KHR;
		shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
		shaderGroups.push_back(shaderGroup);

		// Second shader for shadows
        shaderStages.push_back(LoadShader(GetShadersFolderPath() + "raytracing/shadow.rmiss.spv", VK_SHADER_STAGE_MISS_BIT_KHR));
		shaderGroup.generalShader = static_cast<uint32_t>(shaderStages.size()) - 1;
		shaderGroups.push_back(shaderGroup);
	}

    // Closest hit group
    {
        shaderStages.push_back(LoadShader(GetShadersFolderPath() + "raytracing/closesthit.rchit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR));

		VkRayTracingShaderGroupCreateInfoKHR shaderGroup {};
		shaderGroup.sType              = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		shaderGroup.type               = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
		shaderGroup.generalShader      = VK_SHADER_UNUSED_KHR;
		shaderGroup.closestHitShader   = static_cast<uint32_t>(shaderStages.size()) - 1;
		shaderGroup.anyHitShader       = VK_SHADER_UNUSED_KHR;
		shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
		shaderGroups.push_back(shaderGroup);
	}

	VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCI {};
	rayTracingPipelineCI.sType      = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
	rayTracingPipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	rayTracingPipelineCI.pStages    = shaderStages.data();
	rayTracingPipelineCI.groupCount = static_cast<uint32_t>(shaderGroups.size());
	rayTracingPipelineCI.pGroups    = shaderGroups.data();
	rayTracingPipelineCI.maxPipelineRayRecursionDepth = 2;
	rayTracingPipelineCI.layout                       = pipelineLayout_;
	vkCreateRayTracingPipelinesKHR(device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineCI, nullptr, &pipeline_);
}

void VkRenderer::CreateShaderBindingTables()
{
	const std::uint32_t handleSize = rayTracingPipelineProperties.shaderGroupHandleSize;
	const std::uint32_t handleSizeAligned =
		AlignedSize(rayTracingPipelineProperties.shaderGroupHandleSize,
			rayTracingPipelineProperties.shaderGroupHandleAlignment);

	const auto groupCount = static_cast<std::uint32_t>(shaderGroups.size());
    const std::uint32_t sbtSize = groupCount * handleSizeAligned;

    std::vector<uint8_t> shaderHandleStorage(sbtSize);
    vkGetRayTracingShaderGroupHandlesKHR(device, pipeline_, 0, groupCount, sbtSize, shaderHandleStorage.data());

    // We are using two miss shaders
	shaderBindingTables_.Create(1, 2, 1);

	// Copy handles
    std::memcpy(shaderBindingTables_.raygen.mapped, shaderHandleStorage.data(), handleSize);

    // We are using two miss shaders, so we need to get two handles for the miss shader binding table
    std::memcpy(shaderBindingTables_.miss.mapped, shaderHandleStorage.data() + handleSizeAligned, handleSize * 2);
    std::memcpy(shaderBindingTables_.hit.mapped, shaderHandleStorage.data() + handleSizeAligned * 3, handleSize);
}

void VkRenderer::CreateDescriptorSets()
{
	const Mesh& mesh = ModelManagerLocator::get().GetModel(sceneModelId_)->GetMesh(0);

	std::vector<VkDescriptorPoolSize> poolSizes = {
		{VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1},
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1},
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2}};

	VkDescriptorPoolCreateInfo descriptorPoolInfo {};
	descriptorPoolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	descriptorPoolInfo.pPoolSizes    = poolSizes.data();
	descriptorPoolInfo.maxSets       = 1;
	vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool_);

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo {};
	descriptorSetAllocateInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool     = descriptorPool_;
	descriptorSetAllocateInfo.pSetLayouts        = &descriptorSetLayout_;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet_);

	VkWriteDescriptorSetAccelerationStructureKHR descriptorAccelerationStructureInfo {};
    descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
    descriptorAccelerationStructureInfo.accelerationStructureCount = 1;
	descriptorAccelerationStructureInfo.pAccelerationStructures    = &mesh.GetTopLevelAS().handle;

	// The specialized acceleration structure descriptor has to be chained
	VkWriteDescriptorSet accelerationStructureWrite {};
	accelerationStructureWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	accelerationStructureWrite.pNext           = &descriptorAccelerationStructureInfo;
	accelerationStructureWrite.dstSet          = descriptorSet_;
	accelerationStructureWrite.dstBinding      = 0;
	accelerationStructureWrite.descriptorCount = 1;
	accelerationStructureWrite.descriptorType  = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

	VkDescriptorImageInfo storageImageDescriptor{ VK_NULL_HANDLE, storageImage_.view, VK_IMAGE_LAYOUT_GENERAL };
    VkDescriptorBufferInfo vertexBufferDescriptor{ mesh.GetVertexBuffer(), 0, VK_WHOLE_SIZE };
    VkDescriptorBufferInfo indexBufferDescriptor{ mesh.GetIndexBuffer(), 0, VK_WHOLE_SIZE };

    std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
        // Binding 0: Top level acceleration structure
        accelerationStructureWrite,
        // Binding 1: Ray tracing result image
        GenerateWriteDescriptorSet(descriptorSet_, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &storageImageDescriptor),
        // Binding 2: Uniform data
        GenerateWriteDescriptorSet(descriptorSet_, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2, &ubo_.GetDescriptor()),
        // Binding 3: Scene vertex buffer
        GenerateWriteDescriptorSet(descriptorSet_, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3, &vertexBufferDescriptor),
        // Binding 4: Scene index buffer
        GenerateWriteDescriptorSet(descriptorSet_, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4, &indexBufferDescriptor),
    };

	vkUpdateDescriptorSets(device,
		static_cast<uint32_t>(writeDescriptorSets.size()),
		writeDescriptorSets.data(),
		0,
		VK_NULL_HANDLE);
}

void VkRenderer::BuildCommandBuffers()
{
    if (isFramebufferResized_)
    {
        HandleResize();
    }

    for (size_t i = 0; i < inFlightFences_.size(); i++)
    {
        vkDestroyFence(device, inFlightFences_[i], nullptr);
        vkDestroySemaphore(device, availableSemaphores_[i], nullptr);
        vkDestroySemaphore(device, finishedSemaphores_[i], nullptr);
    }

    const std::size_t imageCount = swapchain.GetImageCount();
    availableSemaphores_.resize(imageCount);
    finishedSemaphores_.resize(imageCount);
    inFlightFences_.resize(imageCount);

    VkSemaphoreCreateInfo semaphoreInfo {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkCommandBufferBeginInfo cmdBufInfo {};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	const auto swapChainImages = swapchain.GetImages();
    const auto winSize = Vec2<std::uint32_t>(BasicEngine::GetInstance()->GetConfig().windowSize);
	VkImageSubresourceRange subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    for (std::int32_t i = 0; i < imageCount; ++i)
    {
        VkResult res = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &availableSemaphores_[i]);
        vkCheckError(res, "Could not create semaphore!");

        res = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &finishedSemaphores_[i]);
        vkCheckError(res, "Could not create semaphore!");

        res = vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences_[i]);
        vkCheckError(res, "Could not create fence!");

        commandBuffers_.emplace_back(false);
		commandBuffers_[i].Begin(0);

        /*
            Dispatch the ray tracing commands
        */
        vkCmdBindPipeline(commandBuffers_[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline_);
        vkCmdBindDescriptorSets(commandBuffers_[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipelineLayout_, 0, 1, &descriptorSet_, 0, nullptr);

		VkStridedDeviceAddressRegionKHR emptySbtEntry = {};
		vkCmdTraceRaysKHR(commandBuffers_[i],
			&shaderBindingTables_.raygen.stridedDeviceAddressRegion,
			&shaderBindingTables_.miss.stridedDeviceAddressRegion,
			&shaderBindingTables_.hit.stridedDeviceAddressRegion,
			&emptySbtEntry,
			winSize.x,
			winSize.y,
			1);

		/*
            Copy ray tracing output to swap chain image
        */

		// Prepare current swap chain image as transfer destination
		swapchain.SetImageLayout(commandBuffers_[i],
			i,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			subresourceRange);

		// Prepare ray tracing output image as transfer source
        commandBuffers_[i].SetImageLayout(
            storageImage_.image,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            subresourceRange);

        VkImageCopy copyRegion{};
        copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        copyRegion.srcOffset = { 0, 0, 0 };
        copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        copyRegion.dstOffset = { 0, 0, 0 };
        copyRegion.extent = { winSize.x, winSize.y, 1 };
        vkCmdCopyImage(commandBuffers_[i], storageImage_.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, swapChainImages[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

		// Transition swap chain image back for presentation
		swapchain.SetImageLayout(commandBuffers_[i],
			i,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			subresourceRange);

		// Transition ray tracing output image back to general layout
        commandBuffers_[i].SetImageLayout(
            storageImage_.image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_LAYOUT_GENERAL,
            subresourceRange);

        commandBuffers_[i].End();
    }
}

void VkRenderer::HandleResize()
{
	const auto winSize = Vec2<std::uint32_t>(BasicEngine::GetInstance()->GetConfig().windowSize);

    // Recreate image
    CreateStorageImage(kFormat, { winSize.x, winSize.y, 1 });

    // Update descriptor
    VkDescriptorImageInfo storageImageDescriptor{ VK_NULL_HANDLE, storageImage_.view, VK_IMAGE_LAYOUT_GENERAL };
    VkWriteDescriptorSet resultImageWrite = GenerateWriteDescriptorSet(
		descriptorSet_, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &storageImageDescriptor);
    vkUpdateDescriptorSets(device, 1, &resultImageWrite, 0, VK_NULL_HANDLE);
}
}    // namespace neko::vk
