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
#ifdef NEKO_OPENGL
#include "gl/gl_window.h"

#include <sstream>

#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>

#include "engine/assert.h"
#include "engine/engine.h"

#include "gl/gl_include.h"

#ifdef EASY_PROFILE_USE
#include <easy/profiler.h>
#endif

namespace neko::sdl
{
void OnResizeRenderCommand::Render()
{
	logDebug(fmt::format("Resize window with new size: {}", newWindowSize_.ToString()));
	glViewport(0, 0, newWindowSize_.x, newWindowSize_.y);
}

void GlWindow::Init()
{
#ifdef EASY_PROFILE_USE
	EASY_BLOCK("OPENGLWindowInit");
#endif
	const auto& config = BasicEngine::GetInstance()->GetConfig();
	// Set our OpenGL version.
#ifdef WIN32
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

	// Turn on double buffering with a 24bit Z buffer.
	// You may need to change this to 16 or 32 for your system
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	SdlWindow::Init();
	SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);

	const std::string videoDriver = SDL_GetCurrentVideoDriver();
	logDebug(videoDriver);

	glRenderContext_ = SDL_GL_CreateContext(window_);
	MakeCurrentContext();

	SDL_GL_SetSwapInterval(config.flags & Configuration::VSYNC ? 1 : 0);
    if (const auto errorCode = glewInit(); GLEW_OK != errorCode)
    {
        logError("Failed to initialize GLEW");
        std::terminate();
    }

	InitImGui();
	LeaveCurrentContext();

	Job initRenderJob([this] { MakeCurrentContext(); });
	auto* engine = BasicEngine::GetInstance();
	engine->ScheduleJob(&initRenderJob, JobThreadType::RENDER_THREAD);

	initRenderJob.Join();
}

void GlWindow::InitImGui()
{
#ifdef EASY_PROFILE_USE
	EASY_BLOCK("ImGuiInit");
#endif
    SdlWindow::InitImGui();
    ImGui_ImplSDL2_InitForOpenGL(window_, glRenderContext_);
    ImGui_ImplOpenGL3_Init("#version 300 es");
}

void GlWindow::GenerateUiFrame()
{
#ifdef EASY_PROFILE_USE
	EASY_BLOCK("ImGuiGenerate");
#endif
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(window_);
	ImGui::NewFrame();
}

void GlWindow::SwapBuffer()
{
#ifdef EASY_PROFILE_USE
	EASY_BLOCK("SwapBuffer");
#endif
	SDL_GL_SwapWindow(window_);
}

void GlWindow::Destroy()
{
#ifdef EASY_PROFILE_USE
	EASY_BLOCK("DestroyWindow");
#endif
	Job leaveContext([this] { LeaveCurrentContext(); });
	BasicEngine::GetInstance()->ScheduleJob(&leaveContext, JobThreadType::RENDER_THREAD);
	leaveContext.Join();
	MakeCurrentContext();
	ImGui_ImplOpenGL3_Shutdown();

	// Delete our OpenGL context
	SDL_GL_DeleteContext(glRenderContext_);

	SdlWindow::Destroy();
}

void GlWindow::RenderUi()
{
#ifdef EASY_PROFILE_USE
	EASY_BLOCK("ImGuiRender");
#endif
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GlWindow::OnResize(Vec2u newWindowSize)
{
	onResizeCommand_.SetWindowSize(newWindowSize);
	RendererLocator::get().Render(&onResizeCommand_);
}

void GlWindow::BeforeRenderLoop()
{
	MakeCurrentContext();
	glCheckError();
}

void GlWindow::AfterRenderLoop() { LeaveCurrentContext(); }

void GlWindow::MakeCurrentContext()
{
	SDL_GL_MakeCurrent(window_, glRenderContext_);
	const auto currentContext = SDL_GL_GetCurrentContext();
	std::ostringstream oss;
	oss << "Current Context: " << currentContext << " Render Context: " << glRenderContext_
		<< " from Thread: " << std::this_thread::get_id();
	if (currentContext == nullptr) { oss << "\nSDL Error: " << SDL_GetError(); }
	logDebug(oss.str());
}

void GlWindow::LeaveCurrentContext()
{
	SDL_GL_MakeCurrent(window_, nullptr);
	const auto currentContext = SDL_GL_GetCurrentContext();

	std::ostringstream oss;
	oss << "Leave current context from thread: " << std::this_thread::get_id();
	if (currentContext != nullptr)
	{
		oss << "[Error] After Leave Current Context, context: " << currentContext;
	}
	logDebug(oss.str());
}
}    // namespace neko::sdl

#endif
