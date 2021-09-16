/*
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
 */
#include "sdl_engine/sdl_engine.h"

#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_internal.h>

#include <fmt/format.h>

#include "engine/log.h"
#include "engine/window.h"
#include "math/vector.h"
#include "utils/file_utility.h"

#ifdef NEKO_PROFILE
#include "easy/profiler.h"
#endif

namespace neko::sdl
{
void SdlEngine::Init()
{
#ifdef NEKO_PROFILE
	EASY_BLOCK("Init Sdl Engine");
#endif
	LogDebug("Current path: " + GetCurrentPath());
	jobSystem_.Init();

	assert(window_ != nullptr);
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);
	window_->Init();

	initAction_.Execute();
	inputManager_.Init();
}

void SdlEngine::Destroy()
{
	BasicEngine::Destroy();

	// Shutdown SDL 2
	SDL_Quit();
}

void SdlEngine::ManageEvent()
{
#ifdef NEKO_PROFILE
	EASY_BLOCK("Manage Event");
#endif
	inputManager_.OnPreUserInput();

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT) isRunning_ = false;
		if (event.type == SDL_WINDOWEVENT)
		{
			if (event.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				LogDebug(fmt::format("Windows resized with new size: ({},{})",
					event.window.data1,
					event.window.data2));
				config_.windowSize = Vec2u(event.window.data1, event.window.data2);
				window_->OnResize(config_.windowSize);
			}
		}

		inputManager_.OnEvent(event);
		onEventAction_.Execute(event);
	}

	if (ImGui::GetCurrentContext()) ImGui::GetIO().KeyMods = ImGui::GetMergedKeyModFlags();
}

void SdlEngine::GenerateUiFrame() { BasicEngine::GenerateUiFrame(); }

void SdlEngine::RegisterOnEvent(SdlEventSystemInterface& eventInterface)
{
	onEventAction_.RegisterCallback(
		[&eventInterface](const SDL_Event& event) { eventInterface.OnEvent(event); });
}

}    // namespace neko::sdl
