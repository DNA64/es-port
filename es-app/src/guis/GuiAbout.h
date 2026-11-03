/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#pragma once
#ifndef ES_APP_GUIS_GUI_ABOUT_H
#define ES_APP_GUIS_GUI_ABOUT_H

#include "components/ImageComponent.h"
#include "components/NinePatchComponent.h"
#include "components/TextComponent.h"
#include "GuiComponent.h"

class GuiAbout : public GuiComponent
{
public:
	explicit GuiAbout(Window* window);

	bool input(InputConfig* config, Input input) override;
	std::vector<HelpPrompt> getHelpPrompts() override;

private:
	NinePatchComponent mFrame;
	ImageComponent mLogo;
	TextComponent mVersion;
	TextComponent mCredit;
	TextComponent mControls;
};

#endif // ES_APP_GUIS_GUI_ABOUT_H
