#pragma once
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
#include <array>
#include <condition_variable>
#include <thread>
#include <vector>

#include "engine/jobsystem.h"
#include "engine/log.h"
#include "engine/system.h"
#include "utils/action_utility.h"
#include "utils/service_locator.h"

namespace neko
{
class SyncBuffersInterface;
class Job;
class Window;
constexpr size_t MAX_COMMAND_NMB = 8'192;

/// Abstraction of a graphic command send to the render thread
class RenderCommandInterface
{
public:
    RenderCommandInterface() = default;
    virtual ~RenderCommandInterface() = default;

    virtual void Render() = 0;
};

/// Abstraction of a a graphic command that can be initialized, destroyed and update
/// Warning! Update can be run concurrently to Render!
class RenderProgram : public RenderCommandInterface, public SystemInterface
{};

class RendererInterface
{
public:
	virtual void Render(RenderCommandInterface*)                    = 0;
	virtual void AddPreRenderJob(Job*)                              = 0;
	virtual void RegisterSyncBuffersFunction(SyncBuffersInterface*) = 0;
};

class NullRenderer final : public RendererInterface
{
public:
	void Render([[maybe_unused]] RenderCommandInterface* command) override {};
	void AddPreRenderJob(Job*) override {}
	void RegisterSyncBuffersFunction(SyncBuffersInterface*) override {}
};

class Renderer : public RendererInterface
{
public:
	enum RendererFlag : std::uint8_t
	{
		IS_RUNNING      = 1u << 0u,
		IS_APP_WAITING  = 1u << 1u,
		IS_RENDERING_UI = 1u << 2u,
		IS_APP_SWAPPING = 1u << 3u
	};

	Renderer();
    virtual ~Renderer() = default;

	/// Send the RenderCommand to the queue for next frame
	void Render(RenderCommandInterface* command) override;

	void Destroy();

	void SetFlag(RendererFlag flag);
	std::uint8_t GetFlag() const;
	void SetWindow(Window* window);

	void AddPreRenderJob(Job* job) override;
	Job* GetSyncJob() { return &syncJob_; }
	Job* GetRenderAllJob() { return &renderAllJob_; }
	void ScheduleJobs();
	void ResetJobs();

	virtual void ClearScreen() = 0;

	void RegisterSyncBuffersFunction(
		[[maybe_unused]] SyncBuffersInterface* syncBuffersInterface) override;

protected:
	/// Called from Engine Loop to sync with the Render Loop
    void SyncBuffers();

	/// Run the first job in the queue
	void PreRender();

	virtual void RenderAll();
	virtual void BeforeRender() {}
	virtual void AfterRender() {}

	Action<> syncBuffersAction_;

	Job renderAllJob_;
	Job syncJob_ {[this] { SyncBuffers(); }};

	std::mutex preRenderJobsMutex_;
	std::vector<Job*> preRenderJobs_;

	Window* window_ = nullptr;

	mutable std::mutex statusMutex_;
	std::uint8_t flags_ {IS_RENDERING_UI};

	std::vector<RenderCommandInterface*> currentCommandBuffer_ = {};
	std::vector<RenderCommandInterface*> nextCommandBuffer_    = {};
};

using RendererLocator = Locator<RendererInterface, NullRenderer>;
}    // namespace neko
