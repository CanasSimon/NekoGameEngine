#pragma once
/* ----------------------------------------------------
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
 Author : Canas Simon
 Co-Author : Floreau Luca
 Date : 13.03.2021
---------------------------------------------------------- */
#include "sdl_engine/sdl_engine.h"

#include "aer/ui/ui_image.h"
#include "aer/ui/ui_text.h"

#ifdef NEKO_OPENGL
#include "gl/font.h"
#include "gl/shader.h"
#endif

namespace neko::aer
{
class AerEngine;
constexpr std::size_t MAX_UI_ELEMENTS = 16;
constexpr const char* kRobotoName     = "Roboto-Medium.ttf";
constexpr const char* kDroidSansName  = "droid_sans.ttf";
//-----------------------------------------------------------------------------
// IUiManager
//-----------------------------------------------------------------------------
/// \brief Manage Uis Elements
class IUiManager
{
public:
	/// Add an Ui Image to draw
	/// \param image Pointer of an UiImage
	virtual void AddUiImage(UiImage* image) = 0;

	/// Add an Ui Text to draw
	/// \param text pointer of an uiText
	virtual void AddUiText(UiText* text)    = 0;

protected:
	~IUiManager() = default;
};

//-----------------------------------------------------------------------------
// NullUiManager
//-----------------------------------------------------------------------------
/// \brief Used for the service locator
class NullUiManager final : public IUiManager
{
public:
	void AddUiImage(UiImage*) override {}
	void AddUiText(UiText*) override {}
};

//-----------------------------------------------------------------------------
// UiManager
//-----------------------------------------------------------------------------
/// \brief Manage UiElement
class UiManager final : public SystemInterface,
						public sdl::SdlEventSystemInterface,
						public IUiManager
{
public:
	explicit UiManager(AerEngine& aerEngine);

	void Init() override;
	void Update(seconds dt) override;
	void Render(std::uint8_t playerNmb);
	void Destroy() override;

	void OnEvent(const SDL_Event& event) override;

	void AddUiImage(UiImage*) override;
	void AddUiText(UiText* text) override;

	void SetWindowSize(Vec2f windowSize);

private:
    /// Return the fontId of one of the loaded font
	FontId GetFontId(FontLoaded fontLoaded) const;

	AerEngine& aerEngine_;
	Job preRender_;

	Vec2f windowSize_;
	Mat4f projection_;

	//Font
#ifdef NEKO_OPENGL
	gl::FontManager fontManager_;
#endif
	FontId robotoId_                = INVALID_FONT_ID;
	FontId droidSansId_             = INVALID_FONT_ID;

	//UiImage
#ifdef NEKO_OPENGL
	gl::Shader uiImageShader_;
#endif
	std::vector<UiImage*> uiImages_ {};
	std::vector<UiText*> uiTexts_ {};
};

using UiManagerLocator = Locator<IUiManager, NullUiManager>;
} // namespace neko::aer
