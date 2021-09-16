#pragma once
#include "vk/models/mesh.h"

namespace neko::vk
{
class MeshQuad final : public Mesh
{
public:
	static std::unique_ptr<MeshQuad> Create(Vec2f extent = Vec2f(kDefaultExtent, kDefaultExtent))
	{
		return std::make_unique<MeshQuad>(true, extent);
	}

	explicit MeshQuad(bool load, Vec2f extent = Vec2f(kDefaultExtent, kDefaultExtent))
	   : Mesh(), extent_(extent)
	{
		if (load) { MeshQuad::Load(); }
	}

	~MeshQuad() override = default;

	void Load()
	{
		const std::vector<Vertex> vertices = {
			Vertex(Vec3f(-extent_.x, -extent_.y, 0.0f), Vec3f(0.0f, 0.0f, 0.0f), Vec2f(0.0f, 0.0f)),
			Vertex(Vec3f(extent_.x, -extent_.y, 0.0f), Vec3f(0.0f, 0.0f, 0.0f), Vec2f(1.0f, 0.0f)),
			Vertex(Vec3f(extent_.x, extent_.y, 0.0f), Vec3f(0.0f, 0.0f, 0.0f), Vec2f(1.0f, 1.0f)),
			Vertex(Vec3f(-extent_.x, extent_.y, 0.0f), Vec3f(0.0f, 0.0f, 0.0f), Vec2f(0.0f, 1.0f)),
		};

		const std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};

		InitData(vertices, indices);
	}

private:
	Vec2f extent_;

	inline static const float kDefaultExtent = 0.5f;
};
}
