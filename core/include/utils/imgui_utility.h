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

 Author : Simon Canas
 Co-Author :
 Date : 09.03.2021
---------------------------------------------------------- */
#include <imgui.h>

#include "graphics/color.h"

namespace ImGui
{
/// Used to determine the side at which to draw the label <br>
/// Also used to differentiate our methods from ImGui's version
enum class LabelPos
{
	RIGHT,
	LEFT
};

/// An internal command to correctly draw the label
void DrawLabel(std::string_view label, LabelPos labelPos = LabelPos::RIGHT);
void EndDrawLabel(LabelPos labelPos = LabelPos::RIGHT);

/// Begin an ImGui window using strings
bool Begin(std::string_view name, bool* isOpen = nullptr, ImGuiWindowFlags flags = 0);

/// A DragFloat with more flexible and better handling of labels
bool DragFloat(std::string_view label,
	float* v,
	LabelPos labelPos      = LabelPos::RIGHT,
	float vSpeed           = 1.0f,
	float vMin             = 0.0f,
	float vMax             = 0.0f,
	const char* format     = "%.3f",
	ImGuiSliderFlags flags = 0);

/// A DragFloat2 with more flexible and better handling of labels
bool DragFloat2(std::string_view label,
	float v[2],
	LabelPos labelPos      = LabelPos::RIGHT,
	float vSpeed           = 1.0f,
	float vMin             = 0.0f,
	float vMax             = 0.0f,
	const char* format     = "%.3f",
	ImGuiSliderFlags flags = 0);

/// A DragFloat3 with more flexible and better handling of labels
bool DragFloat3(std::string_view label,
	float v[3],
	LabelPos labelPos      = LabelPos::RIGHT,
	float vSpeed           = 1.0f,
	float vMin             = 0.0f,
	float vMax             = 0.0f,
	const char* format     = "%.3f",
	ImGuiSliderFlags flags = 0);

/// A DragFloat4 with more flexible and better handling of labels
bool DragFloat4(std::string_view label,
	float v[4],
	LabelPos labelPos      = LabelPos::RIGHT,
	float vSpeed           = 1.0f,
	float vMin             = 0.0f,
	float vMax             = 0.0f,
	const char* format     = "%.3f",
	ImGuiSliderFlags flags = 0);

/// A SliderFloat with more flexible and better handling of labels
bool SliderFloat(const char* label,
	float* v,
	float vMin,
	float vMax,
	LabelPos labelPos      = LabelPos::RIGHT,
	const char* format     = "%.3f",
	ImGuiSliderFlags flags = 0);

/// A SliderFloat2 with more flexible and better handling of labels
bool SliderFloat2(const char* label,
	float v[2],
	float vMin,
	float vMax,
	LabelPos labelPos      = LabelPos::RIGHT,
	const char* format     = "%.3f",
	ImGuiSliderFlags flags = 0);

/// A SliderFloat3 with more flexible and better handling of labels
bool SliderFloat3(const char* label,
	float v[3],
	float vMin,
	float vMax,
	LabelPos labelPos      = LabelPos::RIGHT,
	const char* format     = "%.3f",
	ImGuiSliderFlags flags = 0);

/// A SliderFloat4 with more flexible and better handling of labels
bool SliderFloat4(const char* label,
	float v[4],
	float vMin,
	float vMax,
	LabelPos labelPos      = LabelPos::RIGHT,
	const char* format     = "%.3f",
	ImGuiSliderFlags flags = 0);

/// A special DragBox with a color picker with no alpha channel
bool ColorEdit3(std::string_view label,
	float* col,
	LabelPos labelPos         = LabelPos::RIGHT,
	ImGuiColorEditFlags flags = 0);

/// A special DragBox with a color picker with an alpha channel
bool ColorEdit4(std::string_view label,
	float* col,
	LabelPos labelPos         = LabelPos::RIGHT,
	ImGuiColorEditFlags flags = 0);

///// Draws text using strings <br>
///// Using invisible characters such as '#' can allow for extra padding
//void Text(std::string_view text, ...);
//
///// Draws colored text using strings <br>
///// Using invisible characters such as '#' can allow for extra padding
//void TextColored(const ImVec4& col, std::string_view text, ...);
//
///// Draws text that's centered independently of the current indentation <br>
///// Using invisible characters such as '#' can allow for extra padding
//void TextCentered(std::string_view text, ...);

/// Draws a button that's centered independently of the current indentation
bool ButtonCentered(std::string_view label, const ImVec2& size = ImVec2(0.0f, 0.0f));

/// Draws button with a certain size using strings
bool Button(std::string_view label, const ImVec2& size = ImVec2(0, 0));

/// An InputText that takes a string as argument <br>
/// Allows for dynamic input size
bool InputText(std::string_view label,
	std::string* str,
	LabelPos labelPos         = LabelPos::RIGHT,
	ImGuiInputTextFlags flags = ImGuiInputTextFlags_None);

/// A combo box that takes a vector of strings as argument <br>
/// Allows for more flexible combo size and labels
bool Combo(std::string_view label,
	int* currentItem,
	const std::vector<std::string>* items,
	LabelPos labelPos = LabelPos::RIGHT,
	int heightInItems = -1);

/// A combo box that takes a vector of strings as argument <br>
/// Allows for more flexible combo size and labels <br>
/// Supports unsigned types
bool Combo(std::string_view label,
	unsigned* currentItem,
	const std::vector<std::string>* items,
	LabelPos labelPos = LabelPos::RIGHT,
	int heightInItems = -1);

/// A Checkbox with more flexible labels
bool Checkbox(std::string_view label, bool* v, LabelPos labelPos = LabelPos::RIGHT);

/// Draws a menu item using strings
bool MenuItem(std::string_view label,
	std::string_view shortcut = "",
	bool selected             = false,
	bool enabled              = true);

/// Draws a menu item using strings
bool MenuItem(
	std::string_view label, std::string_view shortcut, bool* isSelected, bool enabled = true);

/// Draws a popup context item using strings
bool BeginPopupContextItem(std::string_view strId = "", ImGuiPopupFlags popupFlags = 1);

/// A structure that replaces the default ImGui one
struct ImGuiTextFilter
{
	ImGuiTextFilter(std::string_view defaultFilter = "");
	bool Draw(std::string_view label = "Filter (inc,-exc)", float width = 0.0f);
	void Build();
	void Clear();

	[[nodiscard]] bool PassFilter(std::string_view text, std::string_view textEnd = "") const;

	[[nodiscard]] bool IsActive() const { return !filters.empty(); }

	// [Internal]
	struct ImGuiTextRange
	{
		const char* b;
		const char* e;

		ImGuiTextRange() = default;
		ImGuiTextRange(const char* b, const char* e) : b(b), e(e) {}
		ImGuiTextRange(std::string_view text) : b(&text.front()), e(&text.back() + 1) {}
		[[nodiscard]] bool Empty() const { return b == e; }
		void Split(char separator, ImVector<ImGuiTextRange>* out) const;
	};

	std::string inputBuf {};
	ImVector<ImGuiTextRange> filters;
	int countGrep {};
};
}    // namespace ImGui
