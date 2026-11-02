/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#include "components/MarqueeTextComponent.h"

#include "renderers/Renderer.h"
#include "ThemeData.h"
#include <algorithm>

namespace
{
	constexpr int WaitMs = 1500;
	constexpr int HoldMs = 2000;
	constexpr float ScrollPixelsPerSecond = 45.0f;
}

MarqueeTextComponent::MarqueeTextComponent(Window* window)
	: GuiComponent(window), mText(window)
{
	mText.setSize(0.0f, 0.0f);
}

void MarqueeTextComponent::restart()
{
	mPhase = Phase::Waiting;
	mPhaseMs = 0;
	mOffset = 0.0f;
}

void MarqueeTextComponent::setText(const std::string& text)
{
	mText.setText(text);
	restart();
}

void MarqueeTextComponent::applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties)
{
	GuiComponent::applyTheme(theme, view, element, properties);
	mText.applyTheme(theme, view, element, properties ^ (ThemeFlags::POSITION | ThemeFlags::SIZE | ThemeFlags::ORIGIN | ThemeFlags::ROTATION));

	mBox = getSize();
	mText.setSize(0.0f, 0.0f);
	mText.setPosition(0.0f, 0.0f);
	restart();
}

void MarqueeTextComponent::update(int deltaTime)
{
	GuiComponent::update(deltaTime);

	const float overflow = mText.getSize().x() - mBox.x();
	const float travel = overflow + mText.getSize().y();
	if (overflow <= 0.0f)
	{
		restart();
		return;
	}

	mPhaseMs += deltaTime;
	switch (mPhase)
	{
	case Phase::Waiting:
		if (mPhaseMs >= WaitMs)
		{
			mPhase = Phase::Scrolling;
			mPhaseMs = 0;
		}
		break;
	case Phase::Scrolling:
		mOffset = std::min(travel, ScrollPixelsPerSecond * static_cast<float>(mPhaseMs) / 1000.0f);
		if (mOffset >= travel)
		{
			mPhase = Phase::Holding;
			mPhaseMs = 0;
		}
		break;
	case Phase::Holding:
		if (mPhaseMs >= HoldMs)
			restart();
		break;
	}
}

void MarqueeTextComponent::render(const Transform4x4f& parentTrans)
{
	if (!isVisible())
		return;

	const Transform4x4f trans = parentTrans * getTransform();
	const Vector2f textSize = mText.getSize();

	Transform4x4f textTrans = trans;
	textTrans.translate(Vector3f(-mOffset, (mBox.y() - textSize.y()) / 2.0f, 0.0f));

	if (textSize.x() <= mBox.x())
	{
		mText.render(textTrans);
		return;
	}

	const float scaleX = trans.r0().x();
	const float scaleY = trans.r1().y();
	const Vector2i position(static_cast<int>(Math::round(trans.translation()[0])), static_cast<int>(Math::round(trans.translation()[1])));
	const Vector2i size(static_cast<int>(Math::round(mBox.x() * scaleX)), static_cast<int>(Math::round(mBox.y() * scaleY)));

	Renderer::pushClipRect(position, size);
	mText.render(textTrans);
	Renderer::popClipRect();
}
