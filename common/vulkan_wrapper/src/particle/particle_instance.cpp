#include "vk/particle/particle_instance.h"

#include "math/transform.h"

#include "sdl_engine/sdl_camera.h"

#include "vk/vk_resources.h"
#include "vk/vk_utilities.h"

#include "Tracy.hpp"

namespace neko::vk
{
VertexInput ParticleInstance::Instance::GetVertexInput(uint32_t baseBinding)
{
	const VkVertexInputBindingDescription bindingDescription = {
		baseBinding, sizeof(Instance), VK_VERTEX_INPUT_RATE_INSTANCE};
	const std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
		{0, baseBinding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Instance, modelMatrix)},
		{1, baseBinding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Instance, modelMatrix) + 16},
		{2, baseBinding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Instance, modelMatrix) + 32},
		{3, baseBinding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Instance, modelMatrix) + 48},
		{4, baseBinding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Instance, colorOffset)},
		{5, baseBinding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Instance, offset)},
		{6, baseBinding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Instance, blend)}};
	return VertexInput(0, bindingDescription, attributeDescriptions);
}

ParticleInstance::ParticleInstance(const MaterialId& material)
   : kMesh_(ModelManagerLocator::get().GetQuad()),
	 materialId_(material),
	 instanceBuffer_(sizeof(ParticleInstance::Instance) * kMaxInstances)
{}

void ParticleInstance::Update(const std::vector<ParticleDrawInfo>& particles)
{
	if (particles.empty()) return;

#ifndef NEKO_RAYTRACING
	Instance* instances;
	instanceBuffer_.MapMemory(reinterpret_cast<char**>(&instances));

	auto viewMatrix = sdl::MultiCameraLocator::get().GenerateViewMatrix(0);
	for (const auto& particle : particles)
	{
		if (instances_ >= kMaxInstances) break;

		auto instance = &instances[instances_];
		instance->modelMatrix =
			Transform3d::Transform(particle.position, Quaternion::Identity, Vec3f(particle.scale));
		for (uint32_t row = 0; row < 3; row++)
			for (uint32_t col = 0; col < 3; col++)
				instance->modelMatrix[row][col] = viewMatrix[col][row];

		instance->colorOffset = {
			particle.colorOffset.r, particle.colorOffset.g, particle.colorOffset.b, 1};
		instance->offset = {particle.imageOffset1.x,
			particle.imageOffset1.y,
			particle.imageOffset2.x,
			particle.imageOffset2.y};
		instance->blend  = {
            particle.imageBlendFactor,
            particle.alpha,
            static_cast<float>(1)    //TODO Remove the '1' and use particle.numberOfRows
        };

		instances_++;
	}

	instanceBuffer_.UnmapMemory();
#else
	auto viewMatrix = sdl::MultiCameraLocator::get().GenerateViewMatrix(0);
	std::vector<Mat4f> matrices;
	matrices.reserve(particles.size());
	for (const auto& particle : particles)
	{
		if (instances_ >= kMaxInstances) break;

		//auto instance = &instances[instances_];
		Mat4f mat =
			Transform3d::Transform(particle.position, Quaternion::Identity, Vec3f(particle.scale));
		for (uint32_t row = 0; row < 3; row++)
			for (uint32_t col = 0; col < 3; col++)
				mat[row][col] = viewMatrix[col][row];

		matrices.push_back(mat);

		//instance->colorOffset = {
		//	particle.colorOffset.r, particle.colorOffset.g, particle.colorOffset.b, 1};
		//instance->offset = {particle.imageOffset1.x,
		//	particle.imageOffset1.y,
		//	particle.imageOffset2.x,
		//	particle.imageOffset2.y};
		//instance->blend  = {
        //    particle.imageBlendFactor,
        //    particle.alpha,
        //    static_cast<float>(1)    //TODO Remove the '1' and use particle.numberOfRows
        //};

		instances_++;
	}

	instances_ = 0;

	CreateTopLevelAS(matrices);
#endif
}

bool ParticleInstance::CmdRender(
	const CommandBuffer& commandBuffer, UniformHandle& uniformScene, Material& material)
{
	if (instances_ == 0) return false;

	// Check if we are in the correct pipeline stage.
	const auto& materialPipeline = material.GetPipelineMaterial();

	// Binds the material pipeline.
	if (!material.BindPipeline(commandBuffer)) return false;

	const auto& pipeline = materialPipeline.GetPipeline();

	// Updates descriptors.
	descriptorSet_.Push(kUboSceneHash, uniformScene);
	descriptorSet_.Push(kDepthHash, VkResources::Inst->GetAttachment("depth"));
	descriptorSet_.PushDescriptorData(material.ExportDescriptorData());
	if (!descriptorSet_.Update(pipeline)) return false;

	// Draws the instanced objects.
	descriptorSet_.BindDescriptor(commandBuffer, pipeline);

	VkBuffer vertexBuffers[2] = {kMesh_->GetVertexBuffer(), instanceBuffer_};
	VkDeviceSize offsets[2]   = {0, 0};

    //Bind buffers
	vkCmdBindVertexBuffers(commandBuffer, 0, 2, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, kMesh_->GetIndexBuffer(), 0, Mesh::GetIndexType());

    //Draw the instances
	vkCmdDrawIndexed(commandBuffer, kMesh_->GetIndexCount(), instances_, 0, 0, 0);

	instances_ = 0;
	return true;
}

#ifdef NEKO_RAYTRACING
void ParticleInstance::CreateTopLevelAS(const std::vector<Mat4f>& instances)
{
	auto* vkObj = vk::VkResources::Inst;
	std::vector<VkAccelerationStructureInstanceKHR> vkInstances {};
	vkInstances.reserve(instances.size());

	const Mesh* mesh = ModelManagerLocator::get().GetQuad();
	for (const auto& instance : instances)
	{
		VkAccelerationStructureInstanceKHR inst {};
		inst.transform                              = ToVkMat4(instance);
		inst.instanceCustomIndex                    = 0;
		inst.mask                                   = 0xFF;
		inst.instanceShaderBindingTableRecordOffset = 0;
		inst.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		inst.accelerationStructureReference = mesh->GetBottomLevelAS().deviceAddress;
		vkInstances.push_back(inst);
	}

	VkAccelerationStructureGeometryKHR accelerationStructureGeometry {};
	accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	accelerationStructureGeometry.flags        = VK_GEOMETRY_OPAQUE_BIT_KHR;
	accelerationStructureGeometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
	accelerationStructureGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
	if (!vkInstances.empty())
	{
		Buffer instanceBuffer(sizeof(VkAccelerationStructureInstanceKHR) * vkInstances.size(),
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
				VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			vkInstances.data());

		VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress {};
		instanceDataDeviceAddress.deviceAddress = GetBufferDeviceAddress(instanceBuffer);
		accelerationStructureGeometry.geometry.instances.data = instanceDataDeviceAddress;
	}
	else
	{
		VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress {};
		instanceDataDeviceAddress.deviceAddress               = 0;
		accelerationStructureGeometry.geometry.instances.data = instanceDataDeviceAddress;
	}

	// Get size info
	VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo {};
	accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationStructureBuildGeometryInfo.type  = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	accelerationStructureBuildGeometryInfo.geometryCount = 1;
	accelerationStructureBuildGeometryInfo.pGeometries   = &accelerationStructureGeometry;

	const std::uint32_t instanceCount = vkInstances.size();
	VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo {};
	accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	VkResources::vkGetAccelerationStructureBuildSizesKHR(vkObj->device,
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&accelerationStructureBuildGeometryInfo,
		&instanceCount,
		&accelerationStructureBuildSizesInfo);

	topLevelAs_.Create(VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, accelerationStructureBuildSizesInfo);

	// Create a small scratch buffer used during build of the top level acceleration structure
	ScratchBuffer scratchBuffer(accelerationStructureBuildSizesInfo.buildScratchSize);

	VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo {};
	accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationBuildGeometryInfo.type  = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	accelerationBuildGeometryInfo.mode  = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	accelerationBuildGeometryInfo.dstAccelerationStructure  = topLevelAs_.handle;
	accelerationBuildGeometryInfo.geometryCount             = 1;
	accelerationBuildGeometryInfo.pGeometries               = &accelerationStructureGeometry;
	accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

	VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo {};
	accelerationStructureBuildRangeInfo.primitiveCount  = instanceCount;
	accelerationStructureBuildRangeInfo.primitiveOffset = 0;
	accelerationStructureBuildRangeInfo.firstVertex     = 0;
	accelerationStructureBuildRangeInfo.transformOffset = 0;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

	// Build the acceleration structure on the device via a one-time command buffer submission
	// Some implementations may support acceleration structure building on the host (VkPhysicalDeviceAccelerationStructureFeaturesKHR->accelerationStructureHostCommands), but we prefer device builds
	CommandBuffer commandBuffer(true);
	VkResources::vkCmdBuildAccelerationStructuresKHR(commandBuffer,
		1,
		&accelerationBuildGeometryInfo,
		accelerationBuildStructureRangeInfos.data());
	commandBuffer.SubmitIdle();

	scratchBuffer.Destroy();
}
#endif
}