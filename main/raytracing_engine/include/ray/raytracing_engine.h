#pragma once
#include "ray/draw_system.h"

namespace neko
{
class RayEngine final : public sdl::SdlEngine
{
public:
	RayEngine() = delete;
	explicit RayEngine(const FilesystemInterface& filesystem, Configuration* config = nullptr);

	void Init() override;
    void Destroy() override;

    void GenerateUiFrame() override;

private:
    DrawSystem drawSystem_;
};
}    // namespace neko::vk