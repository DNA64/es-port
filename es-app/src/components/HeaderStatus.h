/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#pragma once
#ifndef ES_APP_COMPONENTS_HEADER_STATUS_H
#define ES_APP_COMPONENTS_HEADER_STATUS_H

#include "GuiComponent.h"
#include "ThemeData.h"
#include "components/DiskSpaceIndicator.h"
#include <memory>
#include <string>

class HeaderStatus : public GuiComponent
{
public:
	explicit HeaderStatus(Window* window);

	void applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, bool defaultLayout);

	void update(int deltaTime) override;
	void render(const Transform4x4f& parentTrans) override;

private:
	DiskSpaceIndicator mDisk;
	DiskSpaceIndicator mNand;
	bool mDiskShown = false;
	bool mNandShown = false;
};

#endif // ES_APP_COMPONENTS_HEADER_STATUS_H
