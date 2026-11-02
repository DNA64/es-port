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

namespace
{
	constexpr int StartHoldMs = 2500;
	constexpr float LinesPerSecond = 0.9f;
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
	mHoldMs = 0;
	mOffset = 0.0f;
}

bool PagedTextComponent::scrolls() const
{
	return mLineHeight > 0.0f && mText.getSize().y() > static_cast<float>(mVisibleLines) * mLineHeight + 1.0f;
}

float PagedTextComponent::cycleHeight() const
{
	return mText.getSize().y() + mLineHeight;
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

	if (!scrolls())
		return;

	if (mHoldMs < StartHoldMs)
	{
		mHoldMs += deltaTime;
		return;
	}

	mOffset += mLineHeight * LinesPerSecond * static_cast<float>(deltaTime) / 1000.0f;
	const float cycle = cycleHeight();
	if (mOffset >= cycle)
		mOffset -= cycle;
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

	if (scrolls())
	{
		Transform4x4f wrapTrans = trans;
		wrapTrans.translate(Vector3f(0.0f, cycleHeight() - mOffset, 0.0f));
		mText.render(wrapTrans);
	}
	Renderer::popClipRect();
}
