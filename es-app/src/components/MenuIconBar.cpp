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
	constexpr float FrameBleed = 4.0f;
	constexpr int FrameCorner = 10;
}

MenuIconBar::MenuIconBar(Window* window) : GuiComponent(window), mFrame(window, ":/menu_selected.png")
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

void MenuIconBar::applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view)
{
	const ThemeData::ThemeElement* elem = theme->getElement(view, "menuIcons", "container");
	mThemed = elem != nullptr && elem->has("pos") && elem->has("size");
	if (mThemed)
	{
		const Vector2f screen(static_cast<float>(Renderer::getScreenWidth()), static_cast<float>(Renderer::getScreenHeight()));
		mBoxPos = elem->get<Vector2f>("pos") * screen;
		mBoxSize = elem->get<Vector2f>("size") * screen;
	}
	layout();
}

void MenuIconBar::layout()
{
	const float width = static_cast<float>(Renderer::getScreenWidth());
	const float height = static_cast<float>(Renderer::getScreenHeight());
	const float count = static_cast<float>(mIcons.size());
	const float size = mThemed ? mBoxSize.y() : height * IconSizeRatio;
	const float step = mThemed ? mBoxSize.x() / count : height * IconStepRatio;
	const float left = mThemed ? mBoxPos.x() : width * FirstCenterRatio - step / 2;
	const float centerY = mThemed ? mBoxPos.y() + mBoxSize.y() / 2 : height * IconCenterRatio;
	const float bleed = mThemed ? 0.0f : FrameBleed;

	for (std::size_t index = 0; index < mIcons.size(); ++index)
	{
		mIcons[index]->setResize(size, size);
		mIcons[index]->setPosition(left + step * (static_cast<float>(index) + 0.5f), centerY);
	}

	const float boxWidth = mThemed ? size * 1.43f : step * 0.9f;
	const float boxHeight = mThemed ? size * 1.28f : step * 0.8f;
	const float centerX = left + step * (static_cast<float>(mSelected) + 0.5f);
	mFrame.setSize(boxWidth + 2 * bleed, boxHeight + 2 * bleed);
	mFrame.setPosition(centerX - boxWidth / 2 - bleed, centerY - boxHeight / 2 - bleed);
}

void MenuIconBar::render(const Transform4x4f& parentTrans)
{
	if (mFrameVisible)
		mFrame.render(parentTrans);
	for (const auto& icon : mIcons)
		icon->render(parentTrans);
}
