#pragma once
#include "math/matrix.h"

#include "vk/buffers/instance_buffer.h"
#include "vk/buffers/uniform_handle.h"
#include "vk/descriptors/descriptor_handle.h"
#include "vk/material/particle_material.h"
#include "vk/models/mesh.h"
#include "vk/particle/particle.h"

namespace neko::vk
{
constexpr std::string_view kDepthName = "depthMap";
constexpr StringHash kDepthHash       = HashString(kDepthName);
class ParticleInstance
{
public:
	class Instance
	{
	public:
		static VertexInput GetVertexInput(uint32_t baseBinding = 0);

		Mat4f modelMatrix;
		Vec4f colorOffset;
		Vec4f offset;
		Vec3f blend;
	};

	explicit ParticleInstance(const MaterialId& material);

	void Update(const std::vector<ParticleDrawInfo>& particles);

	bool CmdRender(
		const CommandBuffer& commandBuffer, UniformHandle& uniformScene, Material& material);

	[[nodiscard]] const Mesh* GetMesh() const { return kMesh_; }
	[[nodiscard]] MaterialId GetMaterialId() const { return materialId_; }

#ifdef NEKO_RAYTRACING
	void CreateTopLevelAS(const std::vector<Mat4f>& instances = {});

	[[nodiscard]] const AccelerationStructure& GetTopLevelAS() const { return topLevelAs_; }
#endif

private:
	const Mesh* kMesh_;
    MaterialId materialId_;

	uint32_t instances_ = 0;

	DescriptorHandle descriptorSet_;
	InstanceBuffer instanceBuffer_;

	static const uint32_t kMaxInstances = 10'000;

#ifdef NEKO_RAYTRACING
	AccelerationStructure topLevelAs_;
#endif
};
}    // namespace neko::vk
