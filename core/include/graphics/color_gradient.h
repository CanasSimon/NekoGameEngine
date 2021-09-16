#pragma once
#include "graphics/color.h"

namespace neko
{
constexpr std::uint8_t kMaxColorMarks = 255;
class ColorGradient
{
public:
	struct ColorMark
	{
		bool operator==(const ColorMark& other) const
		{
			return color == other.color && position == other.position;
		}

		bool operator!=(const ColorMark& other) const { return !(*this == other); }

		Color4 color   = Color::white;
		float position = 0.0f;    // From 0 to 1
	};

	ColorGradient()
    {
        AddMark(0, Color::white);
        AddMark(1, Color::white);
	}

	~ColorGradient() = default;

	bool operator==(const ColorGradient& other) const
	{
		return marks_ == other.marks_ && colors_ == other.colors_;
	}

	bool operator!=(const ColorGradient& other) const { return !(*this == other); }

	[[nodiscard]] Color4 GetColorAt(float position) const
	{
		float pos = position;
		if (pos < 0.0f) pos = 0.0f;
		if (pos > 1.0f) pos = 1.0f;

		return colors_[pos * (kMaxColorMarks - 1)];
	}

	void AddMark(float position, Color4 color)
    {
		float pos = position;
		if (pos < 0.0f) pos = 0.0f;
		if (pos > 1.0f) pos = 1.0f;

		ColorMark newMark;
		newMark.position = pos;
		newMark.color    = color;
		marks_.push_back(newMark);

		RefreshColors();
	}

	void RemoveMark(ColorMark mark)
	{
		const auto it = std::find(marks_.begin(), marks_.end(), mark);
		marks_.erase(it);
		RefreshColors();
	}

	[[nodiscard]] const std::vector<ColorMark>& GetMarks() const { return marks_; }

	void SetMarks(const std::vector<ColorMark>& marks)
    {
        marks_ = marks;
        RefreshColors();
    }

private:
	void RefreshColors()
	{
		std::sort(marks_.begin(),
			marks_.end(),
			[](const ColorMark a, const ColorMark b) { return a.position < b.position; });

		for (int i = 0; i < kMaxColorMarks; ++i)
		{
			colors_[i] =
				ComputeColorAt(static_cast<float>(i) / (kMaxColorMarks - 1.0f), colors_[i]);
		}
	}

	[[nodiscard]] Color4 ComputeColorAt(float position, Color4 color) const
	{
		Color4 newColor = color;

		float pos = position;

		if (pos < 0.0f) pos = 0.0f;
		if (pos > 1.0f) pos = 1.0f;

		ColorMark lower = marks_[0];
		ColorMark upper = marks_[marks_.size() - 1];

		for (const ColorMark mark : marks_)
		{
			if (mark.position < pos)
			{
				if (lower.position < mark.position) { lower = mark; }
			}

			if (mark.position >= pos)
			{
				if (upper.position > mark.position) { upper = mark; }
			}
		}

		if (upper == lower) { newColor = upper.color; }
		else
		{
			const float distance = upper.position - lower.position;
			const float delta    = (pos - lower.position) / distance;

			//lerp
			newColor[0] = (1.0f - delta) * lower.color[0] + delta * upper.color[0];
			newColor[1] = (1.0f - delta) * lower.color[1] + delta * upper.color[1];
			newColor[2] = (1.0f - delta) * lower.color[2] + delta * upper.color[2];
			newColor[3] = (1.0f - delta) * lower.color[3] + delta * upper.color[3];
		}

		return newColor;
	}

	std::vector<ColorMark> marks_ {};    //TODO Change to std::array
	std::array<Color4, kMaxColorMarks> colors_ {};
};
}
