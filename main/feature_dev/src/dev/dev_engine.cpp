#include "dev/dev_engine.h"
#ifdef NEKO_PROFILE
    #include <easy/profiler.h>
#endif

namespace neko::dev
{
DevEngine::DevEngine(const FilesystemInterface& filesystem, Configuration* config)
   : SdlEngine(filesystem, *config)
{
#ifdef NEKO_PROFILE
	EASY_BLOCK("DevEngine::Constructor");
#endif
	RegisterSystem(sampleBrowser_);
	RegisterOnEvent(sampleBrowser_);
	RegisterOnDrawUi(sampleBrowser_);
}

void DevEngine::Init()
{
#ifdef NEKO_PROFILE
    EASY_BLOCK("DevEngine::Init");
#endif
	SdlEngine::Init();
}

void DevEngine::Destroy()
{
	sampleBrowser_.Destroy();
	SdlEngine::Destroy();
}

void DevEngine::ManageEvent() { SdlEngine::ManageEvent(); }

void DevEngine::GenerateUiFrame()
{
#ifdef NEKO_PROFILE
	EASY_BLOCK("DevEngine::GenerateUiFrame");
#endif
	window_->GenerateUiFrame();
	drawImGuiAction_.Execute();
}
}    // namespace neko::dev
