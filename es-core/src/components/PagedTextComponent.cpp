/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#include "components/PagedTextComponent.h"

#include "renderers/Renderer.h"
#include "ThemeData.h"
#include <algorithm>
#include <cmath>

namespace
{
	constexpr int HoldMs = 2800;
	constexpr int SlideMs = 260;
	constexpr float LineSpacing = 1.25f;
}

PagedTextComponent::PagedTextComponent(Window* window)
	: GuiComponent(window), mText(window)
{
	mText.setLineSpacing(LineSpacing);
}

void PagedTextComponent::layout()
{
	mBox = getSize();
	mText.setLineSpacing(LineSpacing);
	mText.setSize(mBox.x(), 0.0f);
	mText.setPosition(0.0f, 0.0f);

	const std::shared_ptr<Font> font = mText.getFont();
	mLineHeight = font ? font->getHeight(LineSpacing) : 0.0f;
	mVisibleLines = mLineHeight > 0.0f ? std::max(1, static_cast<int>(mBox.y() / mLineHeight)) : 1;
	restart();
}

void PagedTextComponent::onSizeChanged()
{
	GuiComponent::onSizeChanged();
	layout();
}

void PagedTextComponent::setText(const std::string& text)
{
	mText.setText(text);
	restart();
}

void PagedTextComponent::restart()
{
	mPage = 0;
	mPhase = Phase::Holding;
	mPhaseMs = 0;
	mOffset = 0.0f;
	mSlideFrom = 0.0f;
}

int PagedTextComponent::pageCount() const
{
	if (mLineHeight <= 0.0f)
		return 1;
	const int lines = static_cast<int>(std::ceil(mText.getSize().y() / mLineHeight - 0.01f));
	return std::max(1, (lines + mVisibleLines - 1) / mVisibleLines);
}

void PagedTextComponent::applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties)
{
	GuiComponent::applyTheme(theme, view, element, properties);
	mText.applyTheme(theme, view, element, properties ^ (ThemeFlags::POSITION | ThemeFlags::SIZE | ThemeFlags::ORIGIN | ThemeFlags::ROTATION));
	layout();
}

void PagedTextComponent::update(int deltaTime)
{
	GuiComponent::update(deltaTime);

	const int pages = pageCount();
	if (pages <= 1)
		return;

	mPhaseMs += deltaTime;
	if (mPhase == Phase::Holding)
	{
		if (mPhaseMs < HoldMs)
			return;

		mPhase = Phase::Sliding;
		mPhaseMs = 0;
		mSlideFrom = mOffset;
		return;
	}

	const int nextPage = (mPage + 1) % pages;
	const float target = static_cast<float>(nextPage * mVisibleLines) * mLineHeight;
	const float progress = std::min(1.0f, static_cast<float>(mPhaseMs) / static_cast<float>(SlideMs));
	mOffset = mSlideFrom + (target - mSlideFrom) * progress;

	if (progress >= 1.0f)
	{
		mPage = nextPage;
		mOffset = target;
		mPhase = Phase::Holding;
		mPhaseMs = 0;
	}
}

void PagedTextComponent::render(const Transform4x4f& parentTrans)
{
	if (!isVisible())
		return;

	const Transform4x4f trans = parentTrans * getTransform();
	Transform4x4f textTrans = trans;
	textTrans.translate(Vector3f(0.0f, -mOffset, 0.0f));

	const float scaleX = trans.r0().x();
	const float scaleY = trans.r1().y();
	const Vector2i position(static_cast<int>(Math::round(trans.translation()[0])), static_cast<int>(Math::round(trans.translation()[1])));
	const Vector2i size(static_cast<int>(Math::round(mBox.x() * scaleX)),
		static_cast<int>(Math::round(static_cast<float>(mVisibleLines) * mLineHeight * scaleY)));

	Renderer::pushClipRect(position, size);
	mText.render(textTrans);
	Renderer::popClipRect();
}
