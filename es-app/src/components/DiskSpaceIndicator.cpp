/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#include "components/DiskSpaceIndicator.h"

#include "renderers/Renderer.h"
#include "resources/Font.h"
#include <cstdio>
#include <sys/statvfs.h>

namespace
{
	constexpr const char* GamesMount = "/media";
	constexpr int RefreshMs = 5000;
	constexpr float RightMarginRatio = 0.02f;
	constexpr float CenterRatio = 0.075f;
	constexpr float IconSizeRatio = 36.0f / 720.0f;
	constexpr float IconGap = 6.0f;
	constexpr unsigned int TextColor = 0xC8C8D9FF;

	std::string freeSpaceLabel()
	{
		struct statvfs stats;
		if(statvfs(GamesMount, &stats) != 0)
			return "?";

		const double bytes = static_cast<double>(stats.f_bavail) * static_cast<double>(stats.f_frsize);
		char text[32];
		if(bytes >= 1e9)
			std::snprintf(text, sizeof text, "%.1f GB", bytes / 1e9);
		else
			std::snprintf(text, sizeof text, "%.0f MB", bytes / 1e6);
		return text;
	}
}

DiskSpaceIndicator::DiskSpaceIndicator(Window* window)
	: GuiComponent(window), mIcon(window), mText(window, "", Font::get(FONT_SIZE_SMALL), TextColor)
{
	const float size = static_cast<float>(Renderer::getScreenHeight()) * IconSizeRatio;

	mIcon.setImage(":/menu_disk.svg");
	mIcon.setOrigin(0.5f, 0.5f);
	mIcon.setResize(size, size);
	refresh();
}

void DiskSpaceIndicator::refresh()
{
	const float width = static_cast<float>(Renderer::getScreenWidth());
	const float height = static_cast<float>(Renderer::getScreenHeight());
	const float iconSize = height * IconSizeRatio;
	const float right = width * (1.0f - RightMarginRatio);
	const std::string label = freeSpaceLabel();
	const float textWidth = mText.getFont()->sizeText(label).x();

	mText.setText(label);
	mText.setSize(textWidth + 2.0f, 0.0f);
	mText.setPosition(right - textWidth - 2.0f, height * CenterRatio - mText.getFont()->getHeight() / 2);
	mIcon.setPosition(right - textWidth - IconGap - iconSize / 2, height * CenterRatio);
	mElapsedMs = 0;
}

void DiskSpaceIndicator::update(int deltaTime)
{
	GuiComponent::update(deltaTime);
	mElapsedMs += deltaTime;
	if(mElapsedMs >= RefreshMs)
		refresh();
}

void DiskSpaceIndicator::render(const Transform4x4f& parentTrans)
{
	mIcon.render(parentTrans);
	mText.render(parentTrans);
}
