/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#pragma once
#ifndef ES_APP_COMPONENTS_SUSPEND_BALLOON_H
#define ES_APP_COMPONENTS_SUSPEND_BALLOON_H

#include "SuspendPoints.h"
#include "GuiComponent.h"
#include "components/ImageComponent.h"
#include "components/NinePatchComponent.h"

class SuspendBalloon : public GuiComponent
{
public:
	explicit SuspendBalloon(Window* window);

	void setPoint(const SuspendPoint& point);
	void setSelected(bool selected);
	bool isShown() const { return mShown; }

	void render(const Transform4x4f& parentTrans) override;

private:
	void layout();

	NinePatchComponent mCard;
	NinePatchComponent mCardSelected;
	ImageComponent mScreen;
	ImageComponent mBadge;
	bool mShown = false;
	bool mSelected = false;
};

#endif // ES_APP_COMPONENTS_SUSPEND_BALLOON_H
