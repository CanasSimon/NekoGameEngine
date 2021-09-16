#include "vk/models/vertex_input.h"

namespace neko::vk
{
VertexInput::VertexInput(std::uint32_t binding,
	const VkVertexInputBindingDescription& bindingDescriptions,
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions)
   : binding_(binding),
	 bindingDescription_(bindingDescriptions),
	 attributeDescription_(std::move(attributeDescriptions))
{}

bool VertexInput::operator<(const VertexInput& other) const { return binding_ < other.binding_; }

Vertex::Vertex(const Vec3f& pos, const Vec3f& norm, const Vec2f& uv)
   : position(pos), normal(norm), texCoords(uv)
{}

bool Vertex::operator==(const Vertex& other) const
{
	return position == other.position && normal == other.normal && texCoords == other.texCoords &&
	       tangent == other.tangent && bitangent == other.bitangent;
}

VertexInput Vertex::GetVertexInput(std::uint32_t binding)
{
	VkVertexInputBindingDescription bindingDescription;
	bindingDescription.binding   = 0;
	bindingDescription.stride    = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	const std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
		{0, binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)},
		{1, binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)},
		{2, binding, VK_FORMAT_R32G32_SFLOAT,    offsetof(Vertex, texCoords)},
		{3, binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, tangent)},
		{4, binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, bitangent)}};

	return VertexInput(binding, bindingDescription, attributeDescriptions);
}
}    // namespace neko::vk