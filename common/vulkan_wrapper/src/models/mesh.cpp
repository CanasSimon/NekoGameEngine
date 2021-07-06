#include "vk/models/mesh.h"

#include "vk/vk_utilities.h"

namespace neko::vk
{
VertexInput Mesh::Instance::GetVertexInput(uint32_t baseBinding)
{
	VkVertexInputBindingDescription bindingDescription {};
	bindingDescription.binding   = baseBinding;
	bindingDescription.stride    = sizeof(Instance);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

	const VkFormat format3 = VK_FORMAT_R32G32B32_SFLOAT;
	const VkFormat format4 = VK_FORMAT_R32G32B32A32_SFLOAT;
	const std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
		{0, baseBinding, format4, offsetof(Instance, modelMatrix)},
		{1, baseBinding, format4, offsetof(Instance, modelMatrix) + sizeof(Vec4f)},
		{2, baseBinding, format4, offsetof(Instance, modelMatrix) + 2 * sizeof(Vec4f)},
		{3, baseBinding, format4, offsetof(Instance, modelMatrix) + 3 * sizeof(Vec4f)},

		{4, baseBinding, format3, offsetof(Instance, normalMatrix)},
		{5, baseBinding, format3, offsetof(Instance, normalMatrix) + sizeof(Vec3f)},
		{6, baseBinding, format3, offsetof(Instance, normalMatrix) + 2 * sizeof(Vec3f)},
	};

	return VertexInput(0, bindingDescription, attributeDescriptions);
}

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<std::uint32_t>& indices) : Mesh()
{
	InitData(vertices, indices);
}

void Mesh::InitData(const std::vector<Vertex>& vertices, const std::vector<std::uint32_t>& indices)
{
	SetVertices(vertices);
	SetIndices(indices);

	aabb_.lowerLeftBound  = Vec3f(std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max());
	aabb_.upperRightBound = Vec3f(std::numeric_limits<float>::min(),
		std::numeric_limits<float>::min(),
		std::numeric_limits<float>::min());

	for (const auto& vertex : vertices)
	{
		aabb_.lowerLeftBound  = Vec3f(std::min(aabb_.lowerLeftBound.x, vertex.position.x),
            std::min(aabb_.lowerLeftBound.y, vertex.position.y),
            std::min(aabb_.lowerLeftBound.z, vertex.position.z));
		aabb_.upperRightBound = Vec3f(std::max(aabb_.upperRightBound.x, vertex.position.x),
			std::max(aabb_.upperRightBound.y, vertex.position.y),
			std::max(aabb_.upperRightBound.z, vertex.position.z));
	}

	radius_ = std::max(aabb_.lowerLeftBound.Magnitude(), aabb_.upperRightBound.Magnitude());

#ifdef NEKO_RAYTRACING
	CreateBottomLevelAS(vertices, indices);

    const Mat4f mat1 = Mat4f::Identity;
    const Mat4f mat2 = Transform3d::Translate(Mat4f::Identity, Vec3f(10.0f, 0.0f, 0.0f));
	const std::vector<Instance> instances = {{mat1}, {mat2}};
	CreateTopLevelAS(instances);
#endif
}

void Mesh::Destroy() const
{
	vertexBuffer_.Destroy();
	if (indexBuffer_) indexBuffer_->Destroy();
}

void Mesh::Init() { InitData(GetVertices(0), GetIndices(0)); }

bool Mesh::DrawCmd(const CommandBuffer& commandBuffer, const std::uint32_t instance) const
{
	VkBuffer vertexBuffers[] = {vertexBuffer_.GetBuffer()};
	VkDeviceSize offsets[]   = {0};

	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, indexBuffer_->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(commandBuffer, indexCount_, instance, 0, 0, 0);

	return true;
}

std::vector<Vertex> Mesh::GetVertices(const std::size_t offset) const
{
	Buffer vertexStaging(vertexBuffer_.GetSize(),
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	CommandBuffer commandBuffer(true);
	{
		VkBufferCopy copyRegion = {};
		copyRegion.size         = vertexStaging.GetSize();
		vkCmdCopyBuffer(commandBuffer, vertexBuffer_, vertexStaging, 1, &copyRegion);
	}
	commandBuffer.SubmitIdle();

	char* verticesMemory;
	std::vector<Vertex> vertices(vertexCount_);
	vertexStaging.MapMemory(&verticesMemory);
	{
		const std::size_t sizeOfSrcT = vertexStaging.GetSize() / vertexCount_;
		for (std::uint32_t i = 0; i < vertexCount_; i++)
		{
			std::memcpy(&vertices[i],
				static_cast<char*>(verticesMemory) + (i * sizeOfSrcT) + offset,
				sizeof(Vertex));
		}
	}
	vertexStaging.UnmapMemory();
	return vertices;
}

void Mesh::SetVertices(const std::vector<Vertex>& vertices)
{
	if (vertexBuffer_.GetBuffer()) vertexBuffer_.Destroy();
    if (vertices.empty()) return;

	vertexCount_ = static_cast<std::uint32_t>(vertices.size());

	const auto vertexStaging = Buffer(sizeof(Vertex) * vertices.size(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		vertices.data());

	vertexBuffer_ = Buffer(vertexStaging.GetSize(),
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
			VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	CommandBuffer commandBuffer(true);
	{
		VkBufferCopy copyRegion = {};
		copyRegion.size         = vertexStaging.GetSize();
		vkCmdCopyBuffer(commandBuffer, vertexStaging, vertexBuffer_, 1, &copyRegion);
	}

	commandBuffer.SubmitIdle();
	vertexStaging.Destroy();
}

std::vector<std::uint32_t> Mesh::GetIndices(const std::size_t offset) const
{
	if (!indexBuffer_) return {};

	Buffer indexStaging(indexBuffer_->GetSize(),
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	CommandBuffer commandBuffer(true);
	{
		VkBufferCopy copyRegion {};
		copyRegion.size = indexStaging.GetSize();
		vkCmdCopyBuffer(commandBuffer, indexBuffer_.value(), indexStaging, 1, &copyRegion);
	}
	commandBuffer.SubmitIdle();

	char* indicesMemory;
	std::vector<std::uint32_t> indices(indexCount_);
	indexStaging.MapMemory(&indicesMemory);
	{
		const std::size_t sizeOfSrcT = indexStaging.GetSize() / indexCount_;
		for (std::uint32_t i = 0; i < indexCount_; i++)
		{
			std::memcpy(&indices[i],
				static_cast<char*>(indicesMemory) + (i * sizeOfSrcT) + offset,
				sizeof(std::uint32_t));
		}
	}
	indexStaging.UnmapMemory();
	return indices;
}

void Mesh::SetIndices(const std::vector<std::uint32_t>& indices)
{
	indexBuffer_.reset();
	indexCount_ = static_cast<std::uint32_t>(indices.size());
	if (indices.empty()) return;

	const auto indexStaging = Buffer(sizeof(std::uint32_t) * indices.size(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		indices.data());

	indexBuffer_.emplace(indexStaging.GetSize(),
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
			VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	CommandBuffer commandBuffer(true);
	{
		VkBufferCopy copyRegion {};
		copyRegion.size = indexStaging.GetSize();
		vkCmdCopyBuffer(commandBuffer, indexStaging, indexBuffer_.value(), 1, &copyRegion);
	}

	commandBuffer.SubmitIdle();
	indexStaging.Destroy();
}

void Mesh::CreateBottomLevelAS(
	const std::vector<Vertex>& vertices, const std::vector<std::uint32_t>& indices)
{
    auto* vkObj = vk::VkResources::Inst;
    VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
    VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};

    vertexBufferDeviceAddress.deviceAddress = GetBufferDeviceAddress(vertexBuffer_);
    indexBufferDeviceAddress.deviceAddress  = GetBufferDeviceAddress(indexBuffer_.value());

    uint32_t numTriangles = static_cast<uint32_t>(indices.size()) / 3;
    uint32_t maxVertex = vertices.size();

    // Build
    VkAccelerationStructureGeometryKHR accelerationStructureGeometry {};
    accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
    accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
    accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    accelerationStructureGeometry.geometry.triangles.vertexData   = vertexBufferDeviceAddress;
    accelerationStructureGeometry.geometry.triangles.maxVertex    = maxVertex;
    accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof(Vertex);
    accelerationStructureGeometry.geometry.triangles.indexType    = VK_INDEX_TYPE_UINT32;
    accelerationStructureGeometry.geometry.triangles.indexData    = indexBufferDeviceAddress;
    accelerationStructureGeometry.geometry.triangles.transformData.deviceAddress = 0;
    accelerationStructureGeometry.geometry.triangles.transformData.hostAddress   = nullptr;

    // Get size info
    VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo {};
    accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    accelerationStructureBuildGeometryInfo.geometryCount = 1;
    accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;

    VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo {};
    buildSizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
    VkResources::vkGetAccelerationStructureBuildSizesKHR(
        vkObj->device,
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        &accelerationStructureBuildGeometryInfo,
        &numTriangles,
        &buildSizeInfo);

	bottomLevelAs_.Create(VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, buildSizeInfo);

    // Create a small scratch buffer used during build of the bottom level acceleration structure
    ScratchBuffer scratchBuffer(buildSizeInfo.buildScratchSize);

    VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo {};
    accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    accelerationBuildGeometryInfo.dstAccelerationStructure = bottomLevelAs_.handle;
    accelerationBuildGeometryInfo.geometryCount = 1;
    accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
    accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

    VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo {};
    accelerationStructureBuildRangeInfo.primitiveCount  = numTriangles;
    accelerationStructureBuildRangeInfo.primitiveOffset = 0;
    accelerationStructureBuildRangeInfo.firstVertex     = 0;
    accelerationStructureBuildRangeInfo.transformOffset = 0;
    std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = {
        &accelerationStructureBuildRangeInfo};

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

void Mesh::CreateTopLevelAS(const std::vector<Instance>& instances)
{
    auto* vkObj = vk::VkResources::Inst;
	std::vector<VkAccelerationStructureInstanceKHR> vkInstances {};
	for (const auto& instance : instances)
	{
        VkAccelerationStructureInstanceKHR inst {};
		inst.transform                              = ToVkMat4(instance.modelMatrix);
		inst.instanceCustomIndex                    = 0;
		inst.mask                                   = 0xFF;
		inst.instanceShaderBindingTableRecordOffset = 0;
		inst.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		inst.accelerationStructureReference = bottomLevelAs_.deviceAddress;
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
}
// namespace neko::vk