#include "vk/models/mesh_instance.h"

#include "vk/material/material_manager.h"

namespace neko::vk
{
ModelInstance::ModelInstance(const ModelId& modelId)
   : modelId_(modelId),
	 instanceBuffer_(sizeof(Mesh::Instance) * kMaxInstances),
	 uniformObject_(false)
#ifdef NEKO_RAYTRACING
	 ,
	 topLevelAs_(vk::ModelManagerLocator::get().GetModel(modelId_)->GetMeshCount())
#endif
{}

std::unique_ptr<ModelInstance> ModelInstance::Create(const ModelId& modelId)
{
	return std::make_unique<ModelInstance>(modelId);
}

void ModelInstance::Update(std::vector<Mat4f>& modelMatrices)
{
	maxInstances_ = kMaxInstances;
	instances_    = 0;
	if (modelMatrices.empty()) return;

#ifndef NEKO_RAYTRACING
    Mesh::Instance* instances;
	instanceBuffer_.MapMemory(reinterpret_cast<char**>(&instances));
	{
		for (auto& modelMatrix : modelMatrices)
		{
			if (instances_ >= maxInstances_) break;

			auto instance          = &instances[instances_];
			instance->modelMatrix  = modelMatrix;
			instance->normalMatrix = Mat3f(modelMatrix).Inverse().Transpose();
			instances_++;
		}
	}

	instanceBuffer_.UnmapMemory();
#else
	const Model* model = ModelManagerLocator::get().GetModel(modelId_);
	for (std::size_t i = 0; i < model->GetMeshCount(); ++i) CreateTopLevelAS(i, modelMatrices);
#endif
}

bool ModelInstance::CmdRender(
	const CommandBuffer& commandBuffer, UniformHandle& uniformScene, UniformHandle& uniformLight)
{
	if (instances_ == 0) return false;    //No instances

	auto& modelManager = ModelManagerLocator::get();
	if (!modelManager.IsLoaded(modelId_)) return false;

	const Model* model          = modelManager.GetModel(modelId_);
	const std::size_t meshCount = model->GetMeshCount();
	for (std::size_t i = 0; i < meshCount; ++i)
	{
		const Mesh& mesh          = model->GetMesh(i);
		auto& materialManager     = MaterialManagerLocator::get();
		DiffuseMaterial& material = materialManager.GetDiffuseMaterial(mesh.GetMaterialId());

		switch (material.GetRenderMode())
		{
			case Material::RenderMode::VK_OPAQUE:
				if (!CmdRenderOpaque(commandBuffer, uniformScene, uniformLight, mesh, material))
					return false;
				break;
			case Material::RenderMode::VK_TRANSPARENT: break;
		}
	}

	return true;
}

bool ModelInstance::CmdRenderOpaque(const CommandBuffer& commandBuffer,
	UniformHandle& uniformScene,
	UniformHandle& uniformLight,
	const Mesh& mesh,
	Material& material)
{
	const MaterialPipeline& materialPipeline = material.GetPipelineMaterial();
	if (!material.BindPipeline(commandBuffer)) return false;

	const GraphicsPipeline& pipeline = materialPipeline.GetPipeline();

	//Push uniforms to shader
	uniformObject_.PushUniformData(material.ExportUniformData());

	//Push texture to shader
	descriptorSet_.Push(kUboSceneHash, uniformScene);
	descriptorSet_.Push(kUboObjectHash, uniformObject_);
	descriptorSet_.Push(kLightsHash, uniformLight);
	descriptorSet_.PushDescriptorData(material.ExportDescriptorData());
	if (!descriptorSet_.Update(pipeline)) return false;

	descriptorSet_.BindDescriptor(commandBuffer, pipeline);

	VkBuffer vertexBuffers[2] = {mesh.GetVertexBuffer(), instanceBuffer_};
	VkDeviceSize offset[2]    = {0, 0};

	//Bind buffers
	vkCmdBindVertexBuffers(commandBuffer, 0, 2, vertexBuffers, offset);
	vkCmdBindIndexBuffer(commandBuffer, mesh.GetIndexBuffer(), 0, Mesh::GetIndexType());

	//Draw the instances
	vkCmdDrawIndexed(commandBuffer, mesh.GetIndexCount(), instances_, 0, 0, 0);

	return true;
}

#ifdef NEKO_RAYTRACING
void ModelInstance::CreateTopLevelAS(std::size_t meshIndex, const std::vector<Mat4f>& instances)
{
    auto* vkObj = vk::VkResources::Inst;
    std::vector<VkAccelerationStructureInstanceKHR> vkInstances {};
	vkInstances.reserve(instances.size());

	const Mesh& mesh = ModelManagerLocator::get().GetModel(modelId_)->GetMesh(meshIndex);
	for (const auto& instance : instances)
    {
		VkAccelerationStructureInstanceKHR inst {};
		inst.transform                              = ToVkMat4(instance);
		inst.instanceCustomIndex                    = 0;
		inst.mask                                   = 0xFF;
		inst.instanceShaderBindingTableRecordOffset = 0;
		inst.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		inst.accelerationStructureReference = mesh.GetBottomLevelAS().deviceAddress;
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

	topLevelAs_[meshIndex].Create(VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, accelerationStructureBuildSizesInfo);

    // Create a small scratch buffer used during build of the top level acceleration structure
    ScratchBuffer scratchBuffer(accelerationStructureBuildSizesInfo.buildScratchSize);

    VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo {};
    accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    accelerationBuildGeometryInfo.type  = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    accelerationBuildGeometryInfo.mode  = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    accelerationBuildGeometryInfo.dstAccelerationStructure  = topLevelAs_[meshIndex].handle;
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
}    // namespace neko::vk