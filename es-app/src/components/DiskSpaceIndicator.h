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
#include "components/TextComponent.h"

class DiskSpaceIndicator : public GuiComponent
{
public:
	explicit DiskSpaceIndicator(Window* window);

	void update(int deltaTime) override;
	void render(const Transform4x4f& parentTrans) override;

private:
	void refresh();

	ImageComponent mIcon;
	TextComponent mText;
	int mElapsedMs = 0;
};

#endif // ES_APP_COMPONENTS_DISK_SPACE_INDICATOR_H
