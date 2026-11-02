/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#include "components/MenuIconBar.h"

#include "renderers/Renderer.h"

namespace
{
	const char* const IconPaths[] = { ":/menu_display.svg", ":/menu_system.svg", ":/menu_network.svg", ":/menu_library.svg", ":/menu_help.svg" };
	constexpr float IconSizeRatio = 48.0f / 720.0f;
	constexpr float IconStepRatio = 0.118f;
	constexpr float IconCenterRatio = 0.075f;
	constexpr float FirstCenterRatio = 0.398f;
	constexpr float FrameBleed = 12.0f;
	constexpr int FrameCorner = 30;
}

MenuIconBar::MenuIconBar(Window* window) : GuiComponent(window), mFrame(window, ":/suspend_frame_sel.png")
{
	mFrame.setCornerSize(FrameCorner, FrameCorner);
	for (const char* path : IconPaths)
	{
		mIcons.emplace_back(new ImageComponent(window));
		mIcons.back()->setImage(path);
		mIcons.back()->setOrigin(0.5f, 0.5f);
	}
	layout();
}

void MenuIconBar::setSelected(int index)
{
	mSelected = index;
	layout();
}

void MenuIconBar::layout()
{
	const float width = static_cast<float>(Renderer::getScreenWidth());
	const float height = static_cast<float>(Renderer::getScreenHeight());
	const float size = height * IconSizeRatio;
	const float step = height * IconStepRatio;
	const float left = width * FirstCenterRatio - step / 2;
	const float centerY = height * IconCenterRatio;

	for (std::size_t index = 0; index < mIcons.size(); ++index)
	{
		mIcons[index]->setResize(size, size);
		mIcons[index]->setPosition(left + step * (static_cast<float>(index) + 0.5f), centerY);
	}

	const float boxWidth = step * 0.9f;
	const float boxHeight = step * 0.8f;
	const float centerX = left + step * (static_cast<float>(mSelected) + 0.5f);
	mFrame.setSize(boxWidth + 2 * FrameBleed, boxHeight + 2 * FrameBleed);
	mFrame.setPosition(centerX - boxWidth / 2 - FrameBleed, centerY - boxHeight / 2 - FrameBleed);
}

void MenuIconBar::render(const Transform4x4f& parentTrans)
{
	for (const auto& icon : mIcons)
		icon->render(parentTrans);
	if (mFrameVisible)
		mFrame.render(parentTrans);
}
