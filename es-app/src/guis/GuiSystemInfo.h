/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#pragma once
#ifndef ES_APP_GUIS_GUI_SYSTEM_INFO_H
#define ES_APP_GUIS_GUI_SYSTEM_INFO_H

#include "components/NinePatchComponent.h"
#include "components/TextComponent.h"
#include "GuiComponent.h"
#include <memory>
#include <string>
#include <vector>

class GuiSystemInfo : public GuiComponent
{
public:
	GuiSystemInfo(Window* window, const std::string& report);

	bool input(InputConfig* config, Input input) override;
	void render(const Transform4x4f& parentTrans) override;
	std::vector<HelpPrompt> getHelpPrompts() override;

private:
	struct Bar
	{
		float y;
		int percent;
	};

	NinePatchComponent mFrame;
	TextComponent mTitle;
	std::vector<std::shared_ptr<TextComponent>> mTexts;
	std::vector<Bar> mBars;
	float mBarX = 0.0f;
	float mBarWidth = 0.0f;
	float mBarHeight = 0.0f;
};

#endif // ES_APP_GUIS_GUI_SYSTEM_INFO_H
