/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#pragma once
#ifndef ES_APP_COMPONENTS_DISK_SPACE_INDICATOR_H
#define ES_APP_COMPONENTS_DISK_SPACE_INDICATOR_H

#include "GuiComponent.h"
#include "components/ImageComponent.h"
#include "ThemeData.h"
#include "components/TextComponent.h"
#include <memory>
#include <string>

class DiskSpaceIndicator : public GuiComponent
{
public:
	DiskSpaceIndicator(Window* window, std::string mountPoint, const std::string& iconPath);

	bool applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& textElement, const std::string& iconElement);
	void useDefaultLayout();
	void placeLeftOf(const DiskSpaceIndicator& neighbour);
	float getLeftEdge() const { return mLeftEdge; }

	void update(int deltaTime) override;
	void render(const Transform4x4f& parentTrans) override;

private:
	void refresh();

	std::string mMountPoint;
	std::string mIconPath;
	bool mThemed = false;
	bool mThemedIcon = false;
	unsigned int mThemedColor = 0;
	ImageComponent mIcon;
	TextComponent mText;
	const DiskSpaceIndicator* mNeighbour = nullptr;
	float mLeftEdge = 0.0f;
	int mElapsedMs = 0;
};

#endif // ES_APP_COMPONENTS_DISK_SPACE_INDICATOR_H
