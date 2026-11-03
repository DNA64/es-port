/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#include "components/HeaderStatus.h"

HeaderStatus::HeaderStatus(Window* window)
	: GuiComponent(window), mDisk(window, "/media", ":/menu_disk.svg"), mNand(window, "/var/lib", ":/menu_nand.svg")
{
	mNand.placeLeftOf(mDisk);
}

void HeaderStatus::applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, bool defaultLayout)
{
	mDiskShown = mDisk.applyTheme(theme, view, "diskSpace", "diskSpaceIcon");
	mNandShown = mNand.applyTheme(theme, view, "nandSpace", "nandSpaceIcon");

	if(!mDiskShown && !mNandShown && defaultLayout)
	{
		mDisk.useDefaultLayout();
		mNand.useDefaultLayout();
		mNand.placeLeftOf(mDisk);
		mDiskShown = true;
		mNandShown = true;
	}
}

void HeaderStatus::update(int deltaTime)
{
	GuiComponent::update(deltaTime);
	if(mDiskShown)
		mDisk.update(deltaTime);
	if(mNandShown)
		mNand.update(deltaTime);
}

void HeaderStatus::render(const Transform4x4f& parentTrans)
{
	if(mDiskShown)
		mDisk.render(parentTrans);
	if(mNandShown)
		mNand.render(parentTrans);
}
