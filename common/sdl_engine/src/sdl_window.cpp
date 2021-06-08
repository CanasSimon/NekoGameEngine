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
#include "sdl_engine/sdl_window.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"

#include "engine/engine.h"
#include "engine/log.h"

#ifdef NEKO_PROFILE
#include "easy/profiler.h"
#endif

namespace neko::sdl
{
void SdlWindow::Init()
{
#ifdef NEKO_PROFILE
	EASY_BLOCK("InitSdlWindow");
#endif
	auto* engine = (SdlEngine*) BasicEngine::GetInstance();
	engine->RegisterOnEvent(*this);

	const auto& config = BasicEngine::GetInstance()->GetConfig();
	flags_             = SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
#if defined(NEKO_OPENGL) && !defined(NEKO_VULKAN)
	flags_ |= SDL_WINDOW_OPENGL;
#elif defined(NEKO_VULKAN)
	flags_ |= SDL_WINDOW_VULKAN;
#endif

	auto windowSize = Vec2i(config.windowSize);
	if (config.flags & Configuration::FULLSCREEN)
	{
		windowSize = Vec2i::zero;
		flags_ |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	window_ = SDL_CreateWindow(config.windowName.c_str(),
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		windowSize.x,
		windowSize.y,
		flags_);

	if (config.flags & Configuration::FULLSCREEN)
	{
		int windowSizeW = 0;
		int windowSizeH = 0;
		SDL_GetWindowSize(window_, &windowSizeW, &windowSizeH);
		windowSize.x = windowSizeW;
		windowSize.y = windowSizeH;
	}

	// Check that everything worked out okay
	if (window_ == nullptr)
	{
		LogError("Unable to create window\n");
		return;
	}
}

void SdlWindow::InitImGui()
{
#ifdef NEKO_PROFILE
	EASY_BLOCK("InitSdlImGui");
#endif
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void) io;

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;    // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;     // Enable Keyboard Gamepad
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
}

void SdlWindow::Destroy()
{
#ifdef NEKO_PROFILE
	EASY_BLOCK("DestroySdlWindow");
#endif
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	// Destroy our window
	SDL_DestroyWindow(window_);
}

void SdlWindow::SwapBuffer() {}

void SdlWindow::RenderUi() { ImGui::Render(); }

void SdlWindow::OnEvent(const SDL_Event& event)
{
#ifdef NEKO_VULKAN

	if (ImGui::GetCurrentContext()) ImGui_ImplSDL2_ProcessEvent(&event);
#else
	ImGui_ImplSDL2_ProcessEvent(&event);
#endif
}
}    // namespace neko::sdl