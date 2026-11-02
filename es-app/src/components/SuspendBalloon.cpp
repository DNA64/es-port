/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#include "components/SuspendBalloon.h"

#include "renderers/Renderer.h"
#include <SDL_timer.h>
#include <cmath>

namespace
{
	constexpr float ScreenAspect = 7.0f / 8.0f;
	constexpr float ThumbWidthRatio = 0.088f;
	constexpr float PaddingRatio = 0.0125f;
	constexpr float MarginRatio = 0.02f;
	constexpr float TopRatio = 0.012f;
	constexpr float BadgeRatio = 0.03f;
	constexpr int CardCorner = 30;
	constexpr unsigned int DimmedShift = 0x8C8CA8FF;
	constexpr float BobAmplitude = 3.0f;
	constexpr float BobPeriodMs = 520.0f;
}

SuspendBalloon::SuspendBalloon(Window* window)
	: GuiComponent(window),
	  mCard(window, ":/suspend_card.png"), mCardSelected(window, ":/suspend_card_sel.png"),
	  mScreen(window), mBadge(window)
{
	mCard.setCornerSize(CardCorner, CardCorner);
	mCardSelected.setCornerSize(CardCorner, CardCorner);
	mBadge.setImage(":/suspend_badge_b.png");
	layout();
}

void SuspendBalloon::layout()
{
	const float screenWidth = static_cast<float>(Renderer::getScreenWidth());
	const float screenHeight = static_cast<float>(Renderer::getScreenHeight());

	const float thumbWidth = screenWidth * ThumbWidthRatio;
	const float thumbHeight = thumbWidth * ScreenAspect;
	const float padding = screenWidth * PaddingRatio;
	const float width = thumbWidth + 2 * padding;
	const float height = thumbHeight + 2 * padding;

	setSize(width, height);
	setPosition(screenWidth - width - screenWidth * MarginRatio, screenHeight * TopRatio, 0);

	mCard.setSize(width, height);
	mCardSelected.setSize(width, height);

	mScreen.setPosition(padding, padding);
	mScreen.setResize(thumbWidth, thumbHeight);

	const float badge = screenWidth * BadgeRatio;
	mBadge.setPosition(-badge * 0.45f, height - badge * 0.8f);
	mBadge.setResize(badge, badge);
}

void SuspendBalloon::setPoint(const SuspendPoint& point)
{
	mShown = point.valid;
	if(mShown)
		mScreen.setImage(point.thumbnail);
}

void SuspendBalloon::setSelected(bool selected)
{
	mSelected = selected;
	mScreen.setColorShift(selected ? 0xFFFFFFFF : DimmedShift);
}

void SuspendBalloon::render(const Transform4x4f& parentTrans)
{
	if(!mShown)
		return;

	Transform4x4f trans = parentTrans * getTransform();
	const float phase = static_cast<float>(SDL_GetTicks()) / BobPeriodMs;
	trans.translate(Vector3f(0.0f, std::sin(phase) * BobAmplitude, 0.0f));

	if(mSelected)
		mCardSelected.render(trans);
	else
		mCard.render(trans);

	mScreen.render(trans);
	if(!mSelected)
		mBadge.render(trans);
}
