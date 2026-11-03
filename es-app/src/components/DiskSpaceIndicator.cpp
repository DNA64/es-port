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
#include "math/Misc.h"
#include "resources/Font.h"
#include <utility>
#include <cstdio>
#include <sys/statvfs.h>

namespace
{
	constexpr int RefreshMs = 5000;
	constexpr float RightMarginRatio = 0.02f;
	constexpr float CenterRatio = 0.075f;
	constexpr float IconSizeRatio = 30.0f / 720.0f;
	constexpr float FontSizeRatio = 0.028f;
	constexpr float IconGap = 6.0f;
	constexpr float GroupGap = 22.0f;
	constexpr double LowSpaceBytes = 20e6;
	constexpr unsigned int TextColor = 0xD0D0D0FF;
	constexpr unsigned int LowSpaceColor = 0xD07070FF;

	unsigned int fontSize()
	{
		return static_cast<unsigned int>(FontSizeRatio * static_cast<float>(Math::min(static_cast<int>(Renderer::getScreenHeight()), static_cast<int>(Renderer::getScreenWidth()))));
	}

	double freeBytes(const std::string& mountPoint)
	{
		struct statvfs stats;
		if(statvfs(mountPoint.c_str(), &stats) != 0)
			return -1.0;
		return static_cast<double>(stats.f_bavail) * static_cast<double>(stats.f_frsize);
	}

	std::string freeSpaceLabel(double bytes)
	{
		if(bytes < 0.0)
			return "?";

		char text[32];
		if(bytes >= 1e9)
			std::snprintf(text, sizeof text, "%.1f GB", bytes / 1e9);
		else
			std::snprintf(text, sizeof text, "%.0f MB", bytes / 1e6);
		return text;
	}
}

DiskSpaceIndicator::DiskSpaceIndicator(Window* window, std::string mountPoint, const std::string& iconPath)
	: GuiComponent(window), mMountPoint(std::move(mountPoint)), mIcon(window), mText(window, "", Font::get(fontSize()), TextColor)
{
	const float size = static_cast<float>(Renderer::getScreenHeight()) * IconSizeRatio;

	mIcon.setImage(iconPath);
	mIcon.setOrigin(0.5f, 0.5f);
	mIcon.setResize(size, size);
	refresh();
}

void DiskSpaceIndicator::placeLeftOf(const DiskSpaceIndicator& neighbour)
{
	mNeighbour = &neighbour;
	refresh();
}

void DiskSpaceIndicator::refresh()
{
	const float width = static_cast<float>(Renderer::getScreenWidth());
	const float height = static_cast<float>(Renderer::getScreenHeight());
	const float iconSize = height * IconSizeRatio;
	const float right = mNeighbour ? mNeighbour->getLeftEdge() - GroupGap : width * (1.0f - RightMarginRatio);
	const double bytes = freeBytes(mMountPoint);
	const std::string label = freeSpaceLabel(bytes);
	const float textWidth = mText.getFont()->sizeText(label).x();

	mText.setText(label);
	mText.setColor(bytes >= 0.0 && bytes < LowSpaceBytes ? LowSpaceColor : TextColor);
	mText.setSize(textWidth + 2.0f, 0.0f);
	mText.setPosition(right - textWidth - 2.0f, height * CenterRatio - mText.getFont()->getHeight() / 2);
	mIcon.setPosition(right - textWidth - IconGap - iconSize / 2, height * CenterRatio);
	mLeftEdge = right - textWidth - IconGap - iconSize;
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
