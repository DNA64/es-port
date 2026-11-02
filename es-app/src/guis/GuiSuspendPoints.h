/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#pragma once
#ifndef ES_APP_GUIS_GUI_SUSPEND_POINTS_H
#define ES_APP_GUIS_GUI_SUSPEND_POINTS_H

#include "SuspendPoints.h"
#include "GuiComponent.h"
#include "components/ImageComponent.h"
#include "components/NinePatchComponent.h"
#include "components/TextComponent.h"
#include <array>
#include <functional>
#include <memory>
#include <string>
#include <vector>

class GuiSuspendPoints : public GuiComponent
{
public:
	enum class Pad { Snes, Nes };

	GuiSuspendPoints(Window* window, std::string title, std::string code, SuspendPoints points, Pad pad,
		std::function<void(int)> onResume, std::function<void()> onChanged);

	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;
	void render(const Transform4x4f& parentTrans) override;
	std::vector<HelpPrompt> getHelpPrompts() override;

private:
	void refresh();
	void saveSelected(SuspendPoints::SaveMode mode);
	void notify(const std::string& text);
	void close();
	void layout();
	bool isSaveKey(InputConfig* config, const Input& input) const;
	const SuspendPoint& selectedPoint() const;

	std::string mTitle;
	std::string mCode;
	SuspendPoints mPoints;
	Pad mPad;
	std::function<void(int)> mOnResume;
	std::function<void()> mOnChanged;

	SuspendPoint mFloating;
	std::array<SuspendPoint, SuspendPoints::MaxSlots> mSlots;
	int mSelected = 1;
	bool mHolding = false;
	int mHeldMs = 0;
	int mNoticeMs = 0;

	NinePatchComponent mPanel;
	NinePatchComponent mCell;
	NinePatchComponent mCellSelected;
	NinePatchComponent mFrame;
	NinePatchComponent mFrameSelected;
	TextComponent mHeading;
	TextComponent mNotice;
	ImageComponent mLock;
	std::vector<std::unique_ptr<ImageComponent>> mShots;
	std::vector<std::unique_ptr<TextComponent>> mLabels;
};

#endif // ES_APP_GUIS_GUI_SUSPEND_POINTS_H
