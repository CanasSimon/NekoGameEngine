#include "aer/aer_engine.h"

#include "engine/resource_locations.h"

#include "sdl_engine/sdl_input.h"

namespace neko::aer
{
UiManager::UiManager(AerEngine& aerEngine)
   : aerEngine_(aerEngine)
#ifdef NEKO_OPENGL
	 ,
	 fontManager_(aerEngine.GetFilesystem())
#endif
{}

void UiManager::Init()
{
	UiManagerLocator::provide(this);
	uiImages_.reserve(MAX_UI_ELEMENTS);
	uiTexts_.reserve(MAX_UI_ELEMENTS);

#ifdef NEKO_OPENGL
	const auto& config = aerEngine_.GetConfig();
	preRender_         = Job {[this, config]()
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            uiImageShader_.LoadFromFile(GetGlShadersFolderPath() + "ui_image.vert",
                GetGlShadersFolderPath() + "ui_image.frag");
            fontManager_.Init();
			droidSansId_ = fontManager_.LoadFont(GetFontsFolderPath() + kDroidSansName, 128);
            robotoId_  = fontManager_.LoadFont(GetFontsFolderPath() + kRobotoName, 128);

            uiImageShader_.BindUbo(gl::kUboUiProjSize, gl::kUboUiProjBinding);

            SetWindowSize(Vec2f(config.windowSize));
            glCheckError();
        }};

	RendererLocator::get().AddPreRenderJob(&preRender_);
#endif
}

void UiManager::Update(seconds)
{
	auto& inputManager = sdl::InputLocator::get();

	// To ensure that visibility is only d once
	for (auto& image : uiImages_) image->UpdateVisibility();
	for (auto& text : uiTexts_) text->UpdateVisibility();
}

void UiManager::Render(std::uint8_t playerNmb)
{
#ifdef NEKO_OPENGL
	const auto& config = aerEngine_.GetConfig();
	SetWindowSize(Vec2f(config.windowSize));
	if (uiImageShader_.GetProgram() == 0) return;

	uiImageShader_.Bind();
	glDisable(GL_DEPTH_TEST);
	{
		const Vec2i winSize = Vec2i(aerEngine_.GetConfig().windowSize);
		glViewport(0, 0, winSize.x, winSize.y);
		for (auto& image : uiImages_)
		{
			auto& textureManager = aerEngine_.GetResourceManagerContainer().textureManager;
			if (image->GetFlags() & UiFlag::INITIALIZED && image->GetFlags() & UiFlag::ENABLED)
			{
				image->Draw(textureManager, playerNmb, uiImageShader_);
			}
			else if (!(image->GetFlags() & UiFlag::INITIALIZED))
			{
				image->Init(textureManager);
				image->AddFlag(UiFlag::INITIALIZED);
			}
		}

		for (auto& text : uiTexts_)
		{
			if (text->GetFlags() & UiFlag::ENABLED)
			{
				text->Draw(fontManager_, GetFontId(text->GetFont()), playerNmb);
			}
		}
	}
	glEnable(GL_DEPTH_TEST);
#endif
}

void UiManager::OnEvent(const SDL_Event& event)
{
	if (event.window.event == SDL_WINDOWEVENT_RESIZED)
	{
		for (auto& element : uiImages_) element->AddFlag(UiFlag::DIRTY);
	}
}

void UiManager::Destroy()
{
	for (auto image : uiImages_) image->Destroy();
	uiImages_.clear();
	uiTexts_.clear();

#ifdef NEKO_OPENGL
	uiImageShader_.Destroy();
	fontManager_.Destroy();
#endif
}

void UiManager::AddUiImage(UiImage* image)
{
	auto it = std::find(uiImages_.begin(), uiImages_.end(), image);
	if (it == uiImages_.end()) uiImages_.push_back(image);
	else
		LogDebug("UiText already draw");
}

void UiManager::AddUiText(UiText* text)
{
	auto it = std::find(uiTexts_.begin(), uiTexts_.end(), text);
	if (it == uiTexts_.end()) uiTexts_.push_back(text);
	else
		LogDebug("UiText already draw");
}

FontId UiManager::GetFontId(FontLoaded fontLoaded) const
{
	switch (fontLoaded)
	{
		case FontLoaded::DROID_SANS: return droidSansId_;
		case FontLoaded::ROBOTO: return robotoId_;
		default: return INVALID_FONT_ID;
	}
}

void UiManager::SetWindowSize(const Vec2f windowSize)
{
	windowSize_ = windowSize;
	projection_ = Transform3d::Orthographic(0.0f, windowSize.x, 0.0f, windowSize.y);

#ifdef NEKO_OPENGL
	uiImageShader_.SetUbo(gl::kUboUiProjSize, 0, &projection_, gl::kUboUiProjBinding);

	fontManager_.SetWindowSize(windowSize);
#endif
}
}    // namespace neko::aer
