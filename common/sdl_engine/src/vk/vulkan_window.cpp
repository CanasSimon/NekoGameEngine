#ifdef NEKO_VULKAN
#include "vk/vulkan_window.h"

#include "engine/log.h"

#ifdef NEKO_PROFILE
#include <easy/profiler.h>
#endif

namespace neko::sdl
{
VulkanWindow::VulkanWindow()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
	Init();
}

void VulkanWindow::Init()
{
#ifdef NEKO_PROFILE
	EASY_BLOCK("VulkanWindowInit");
#endif
	SdlWindow::Init();

#ifdef VALIDATION_LAYERS
	const std::string videoDriver = SDL_GetCurrentVideoDriver();
	LogDebug(videoDriver);
#endif
}

void VulkanWindow::Destroy()
{
#ifdef NEKO_PROFILE
	EASY_BLOCK("DestroyWindow");
#endif

    SDL_DestroyWindow(window_);
}

void VulkanWindow::MinimizedLoop() const
{
	while (SDL_GetWindowFlags(window_) & SDL_WINDOW_MINIMIZED) SDL_PumpEvents();
}
}    // namespace neko::sdl
#endif
