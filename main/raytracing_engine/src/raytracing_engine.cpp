#include "ray/raytracing_engine.h"

#include "utils/file_utility.h"

#include "vk/vk_resources.h"

namespace neko
{
RayEngine::RayEngine(const FilesystemInterface& filesystem, Configuration* config)
    : SdlEngine(filesystem, *config)
{
    RegisterSystem(drawSystem_);
    RegisterOnEvent(drawSystem_);
}

void RayEngine::Init()
{
#ifdef NEKO_PROFILE
    EASY_BLOCK("Init Sdl Engine");
#endif
    LogDebug("Current path: " + GetCurrentPath());
    jobSystem_.Init();

    initAction_.Execute();
    inputManager_.Init();
}

void RayEngine::Destroy()
{
    drawSystem_.Destroy();
    SdlEngine::Destroy();
}

void RayEngine::GenerateUiFrame()
{
    if (ImGui::GetCurrentContext())
        drawImGuiAction_.Execute();
}
}    // namespace neko::vk