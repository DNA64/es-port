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
#include <cmath>

namespace
{
	constexpr int WaitMs = 1500;
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
	mClipLeft = mClipRight = -1.0f;
	if (const ThemeData::ThemeElement* elem = theme->getElement(view, element, "text"))
	{
		if (elem->has("clipLeft"))
			mClipLeft = elem->get<float>("clipLeft");
		if (elem->has("clipRight"))
			mClipRight = elem->get<float>("clipRight");
	}
	mText.setSize(0.0f, 0.0f);
	mText.setPosition(0.0f, 0.0f);
	restart();
}

void MarqueeTextComponent::update(int deltaTime)
{
	GuiComponent::update(deltaTime);

	const float overflow = mText.getSize().x() - mBox.x();
	const float period = mText.getSize().x() + mText.getSize().y() * 2.0f;
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
		mOffset = std::fmod(ScrollPixelsPerSecond * static_cast<float>(mPhaseMs) / 1000.0f, period);
		break;
	}
}

void MarqueeTextComponent::render(const Transform4x4f& parentTrans)
{
	if (!isVisible())
		return;

	const Transform4x4f trans = parentTrans * getTransform();
	const Vector2f textSize = mText.getSize();
	const float period = textSize.x() + textSize.y() * 2.0f;

	Transform4x4f textTrans = trans;
	textTrans.translate(Vector3f(-mOffset, (mBox.y() - textSize.y()) / 2.0f, 0.0f));

	if (textSize.x() <= mBox.x())
	{
		const float spare = mBox.x() - textSize.x();
		const Alignment alignment = mText.getHorizontalAlignment();
		textTrans.translate(Vector3f(alignment == ALIGN_CENTER ? spare / 2.0f : alignment == ALIGN_RIGHT ? spare : 0.0f, 0.0f, 0.0f));
		mText.render(textTrans);
		return;
	}

	const float scaleX = trans.r0().x();
	const float scaleY = trans.r1().y();
	Vector2i position(static_cast<int>(Math::round(trans.translation()[0])), static_cast<int>(Math::round(trans.translation()[1])));
	Vector2i size(static_cast<int>(Math::round(mBox.x() * scaleX)), static_cast<int>(Math::round(mBox.y() * scaleY)));

	const float screenWidth = static_cast<float>(Renderer::getScreenWidth());
	const int right = position.x() + size.x();
	if (mClipLeft >= 0.0f)
		position.x() = static_cast<int>(Math::round(mClipLeft * screenWidth));
	size.x() = (mClipRight > 0.0f ? static_cast<int>(Math::round(mClipRight * screenWidth)) : right) - position.x();

	Renderer::pushClipRect(position, size);
	mText.render(textTrans);
	Transform4x4f secondTrans = textTrans;
	secondTrans.translate(Vector3f(period, 0.0f, 0.0f));
	mText.render(secondTrans);
	Renderer::popClipRect();
}
