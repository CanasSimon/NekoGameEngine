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
#include "vk/buffers/instance_buffer.h"
#include "vk/descriptors/descriptor_handle.h"
#include "vk/models/model_manager.h"

namespace neko::vk
{
constexpr static std::string_view kUboSceneName  = "UboScene";
constexpr static std::string_view kUboObjectName = "UboObject";
constexpr static std::string_view kLightsName    = "Lights";
constexpr static StringHash kUboSceneHash        = HashString(kUboSceneName);
constexpr static StringHash kUboObjectHash       = HashString(kUboObjectName);
constexpr static StringHash kLightsHash          = HashString(kLightsName);
constexpr static std::uint32_t kMaxInstances     = 32;

class ModelInstance
{
public:
	explicit ModelInstance(const ModelId& modelId);

	[[nodiscard]] static std::unique_ptr<ModelInstance> Create(const ModelId& modelId);

	void Update(std::vector<Mat4f>& modelMatrices);

	bool CmdRender(const CommandBuffer& commandBuffer,
		UniformHandle& uniformScene,
		UniformHandle& uniformLight);

	[[nodiscard]] const ModelId& GetModelId() const { return modelId_; }

	void AddInstance(const std::size_t instanceNum = 1) { instances_ += instanceNum; }
	void SetModelId(const ModelId& modelId) { modelId_ = modelId; }

#ifdef NEKO_RAYTRACING
    void CreateTopLevelAS(std::size_t meshIndex, const std::vector<Mat4f>& instances = {});

    [[nodiscard]] const AccelerationStructure& GetTopLevelAS(std::size_t meshIndex) const
	{ return topLevelAs_[meshIndex]; }

    [[nodiscard]] const std::vector<AccelerationStructure>& GetTopLevelASs() const
	{ return topLevelAs_; }

	[[nodiscard]] std::vector<VkAccelerationStructureKHR> GetTopLevelASHandles() const
	{
    	std::vector<VkAccelerationStructureKHR> handles(topLevelAs_.size());
		for (std::size_t i = 0; i < topLevelAs_.size(); ++i)
			handles[i] = topLevelAs_[i].handle;

		return handles;
	}
#endif

private:
	bool CmdRenderOpaque(const CommandBuffer& commandBuffer,
		UniformHandle& uniformScene,
		UniformHandle& uniformLight,
		const Mesh& mesh,
		Material& material);

	ModelId modelId_;

	std::uint32_t maxInstances_ = 0;
	std::uint32_t instances_    = 0;

	DescriptorHandle descriptorSet_;
    InstanceBuffer instanceBuffer_;
	UniformHandle uniformObject_;

#ifdef NEKO_RAYTRACING
    std::vector<AccelerationStructure> topLevelAs_;
#endif
};
}    // namespace neko::vk
